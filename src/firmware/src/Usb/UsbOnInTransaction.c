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
	const uint8_t inBufferLength;
	const uint8_t *inBuffer;
	size_t remaining;
	UsbEpDataStageCallback callback;
	void *context;
	bool isBusy;
};

static struct EndpointContext endpointContexts[] =
{
	{EP_0_LEN},
#if NUM_ENDPOINT_NUMBERS >= 1
	{EP_1_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 2
	{EP_2_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 3
	{EP_3_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 4
	{EP_4_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 5
	{EP_5_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 6
	{EP_6_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 7
	{EP_7_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 8
	{EP_8_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 9
	{EP_9_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 10
	{EP_10_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 11
	{EP_11_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 12
	{EP_12_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 13
	{EP_13_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 14
	{EP_14_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 15
	{EP_15_IN_LEN},
#endif
#if NUM_ENDPOINT_NUMBERS >= 16
	{EP_16_IN_LEN}
#endif
};

bool usbStartSendEpDataStage(uint8_t endpoint, uint8_t *buffer, size_t length, UsbEpDataStageCallback callback, void *context)
{
	if (endpoint == 0 || endpoint > NUM_ENDPOINT_NUMBERS || usbIsInEndpointBusy(endpoint))
		return false;

	bool result = false;
	taskENTER_CRITICAL();
	if (!usbIsInEndpointBusy(endpoint))
	{
		endpointContexts[endpoint].inBuffer = buffer;
		endpointContexts[endpoint].remaining = length;
		endpointContexts[endpoint].callback = callback;
		endpointContexts[endpoint].context = context;
		endpointContexts[endpoint].isBusy = true;
		usbOnInTransaction(endpoint);
		result = true;
	}
	taskEXIT_CRITICAL();
	return result;
}

bool usbIsInEndpointBusy(uint8_t endpoint)
{
	if (endpoint > NUM_ENDPOINT_NUMBERS)
		return false;

	return endpointContexts[endpoint].isBusy || usb_in_endpoint_busy(endpoint);
}

void usbOnInTransaction(uint8_t endpoint)
{
	if (endpoint > NUM_ENDPOINT_NUMBERS || !endpointContexts[endpoint].isBusy)
		return;

	while (!usb_in_endpoint_busy(endpoint))
	{
		size_t transferLength = endpointContexts[endpoint].remaining;
		if (transferLength > endpointContexts[endpoint].inBufferLength)
			transferLength = endpointContexts[endpoint].inBufferLength;

		if (transferLength == 0)
		{
			if (endpointContexts[endpoint].inBuffer)
			{
				usb_send_in_buffer(endpoint, 0);
				endpointContexts[endpoint].inBuffer = (uint8_t *) 0;
			}
			else
			{
				endpointContexts[endpoint].isBusy = false;
				if (endpointContexts[endpoint].callback && endpointContexts[endpoint].callback(endpoint, true, endpointContexts[endpoint].context) < 0)
				{
					usb_halt_ep_in(endpoint);
					usbOnEndpointHalt(endpoint, true);
				}
			}

			return;
		}

		memcpy(usb_get_in_buffer(endpoint), endpointContexts[endpoint].inBuffer, transferLength);
		usb_send_in_buffer(endpoint, transferLength);
		endpointContexts[endpoint].inBuffer += transferLength;
		endpointContexts[endpoint].remaining -= transferLength;
		if (endpointContexts[endpoint].remaining == 0 && transferLength != endpointContexts[endpoint].inBufferLength)
			endpointContexts[endpoint].inBuffer = (uint8_t *) 0;

	}
}
