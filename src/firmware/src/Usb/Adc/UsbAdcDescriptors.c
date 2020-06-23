#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbAdcInterface.h"

static const uint8_t usbAdcReportDescriptors[] =
{
	#include "AdcReportDescriptors.txt"
};

int16_t usbAdcGetReportDescriptor(const void **ptr)
{
	*ptr = usbAdcReportDescriptors;
	return sizeof(usbAdcReportDescriptors);
}

STRING_DESCRIPTOR(usbAdcInterfaceName, 'A', 'D', 'C');

static int16_t usbAdcGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbAdcInterfaceName;
		return *((uint8_t *) &usbAdcInterfaceName);
	}

	return -1;
}

static const struct UsbInInterface(1) usbAdcInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = ADC_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(ADC, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbAdcReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | ADC_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_ADC_IN_LEN,
		.bInterval = 1
	}
};

static uint16_t usbAdcMemcpyInterfaceDescriptor(void *dest)
{
#if ADC_INTERFACE_ENABLED
	memcpy(dest, &usbAdcInterfaceDescriptor, sizeof(usbAdcInterfaceDescriptor));
	return sizeof(usbAdcInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbAdcGetHidDescriptor(const void **ptr)
{
	*ptr = &usbAdcInterfaceDescriptor.hid;
	return sizeof(usbAdcInterfaceDescriptor);
}

const struct UsbInterface usbAdcInterface =
{
	.endpoint = ADC_EP_ID,
	.reportLengthFor = &usbAdcReportLengthFor,
	.getOutputReportBufferFor = &usbAdcGetOutputReportBufferFor,
	.onReportReceived = &usbAdcOnReportReceived,
	.getInputReportFor = NULL,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbAdcMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbAdcGetHidDescriptor,
	.getStringDescriptor = &usbAdcGetStringDescriptor,
	.getReportDescriptor = &usbAdcGetReportDescriptor
};
