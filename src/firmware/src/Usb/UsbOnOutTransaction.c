#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Usb.h"

#include "usb_config.h"
#include "m-stack/include/usb.h"
#include "m-stack/include/usb_ch9.h"
#include "m-stack/include/usb_hid.h"

struct EndpointContext
{
	uint8_t *outBuffer;
	size_t remaining;
	UsbEpDataStageCallback callback;
	void *context;
};

static struct EndpointContext endpointContexts[NUM_ENDPOINT_NUMBERS + 1];

static int8_t usbEp0DataStageCallback(bool dataOk, void *context);

void usbOnOutTransaction(uint8_t endpoint)
{
	if (!usb_out_endpoint_has_data(endpoint))
		return;

	const unsigned char *buffer;
	uint8_t length = usb_get_out_buffer(endpoint, &buffer);
	usb_arm_out_endpoint(endpoint);
	if (endpointContexts[endpoint].remaining == 0)
	{
		if (length > 0)
		{
			uint8_t reportType = 2;
			uint8_t reportId = (uint8_t) buffer[0];
			uint8_t interface = usbMapEndpointToInterface(endpoint);
			if (interface >= NUMBER_OF_INTERFACES)
			{
				usb_halt_ep_out(endpoint);
				usbOnEndpointHalt(endpoint, true);
				return;
			}

			int8_t result = HID_SET_REPORT_CALLBACK(interface, reportType, reportId);
			if (result == MSTACK_NAK)
			{
				usb_halt_ep_out(endpoint); // TODO: WE DON'T REALLY WANT TO HALT HERE, WE WANT TO NAK.  BUT I'M UNSURE HOW TO DO THAT - MAYBE SOME 'UOWN' JIGGERY-POKERY, BUT THAT INVOLVES M-STACK SURGERY (*IF* UOWN WOULD EVEN WORK AS INTENDED)
				usbOnEndpointHalt(endpoint, true);
			}

			if (result == MSTACK_STALL)
			{
				usb_halt_ep_out(endpoint);
				usbOnEndpointHalt(endpoint, true);
			}
		}
	}

	if (endpointContexts[endpoint].remaining > 0)
	{
		if (endpointContexts[endpoint].outBuffer)
			memcpy(endpointContexts[endpoint].outBuffer, buffer, length);

		endpointContexts[endpoint].outBuffer += length;
		if (length <= endpointContexts[endpoint].remaining)
		{
			endpointContexts[endpoint].remaining -= length;
			if (endpointContexts[endpoint].remaining == 0 && endpointContexts[endpoint].callback)
			{
				if (endpointContexts[endpoint].callback(endpoint, true, endpointContexts[endpoint].context) < 0)
				{
					usb_halt_ep_out(endpoint);
					usbOnEndpointHalt(endpoint, true);
				}
			}
		}
		else
		{
			usb_halt_ep_out(endpoint);
			usbOnEndpointHalt(endpoint, true);
		}
	}
}

void usbStartReceiveEpDataStage(uint8_t endpoint, uint8_t *buffer, size_t length, UsbEpDataStageCallback callback, void *context)
{
	if (endpoint > NUM_ENDPOINT_NUMBERS)
		return;

	if (endpoint == 0)
		usb_start_receive_ep0_data_stage((char *) buffer, length, usbEp0DataStageCallback, context);

	endpointContexts[endpoint].outBuffer = buffer;
	endpointContexts[endpoint].remaining = length;
	endpointContexts[endpoint].callback = callback;
	endpointContexts[endpoint].context = context;
}

static int8_t usbEp0DataStageCallback(bool dataOk, void *context)
{
	return endpointContexts[0].callback(0, dataOk, context);
}
