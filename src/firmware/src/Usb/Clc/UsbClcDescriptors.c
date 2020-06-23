#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbClcInterface.h"

static const uint8_t usbClcReportDescriptors[] =
{
	#include "ClcReportDescriptors.txt"
};

int16_t usbClcGetReportDescriptor(const void **ptr)
{
	*ptr = usbClcReportDescriptors;
	return sizeof(usbClcReportDescriptors);
}

STRING_DESCRIPTOR(usbClcInterfaceName, 'C', 'L', 'C');

static int16_t usbClcGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbClcInterfaceName;
		return *((uint8_t *) &usbClcInterfaceName);
	}

	return -1;
}

static const struct UsbInOutInterface(1) usbClcInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = CLC_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(CLC, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbClcReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | CLC_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_CLC_IN_LEN,
		.bInterval = 1
	},
	.ep_out =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_OUT | CLC_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_CLC_OUT_LEN,
		.bInterval = 1
	}
};

static uint16_t usbClcMemcpyInterfaceDescriptor(void *dest)
{
#ifdef CLC_INTERFACE_ENABLED
	memcpy(dest, &usbClcInterfaceDescriptor, sizeof(usbClcInterfaceDescriptor));
	return sizeof(usbClcInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbClcGetHidDescriptor(const void **ptr)
{
	*ptr = &usbClcInterfaceDescriptor.hid;
	return sizeof(usbClcInterfaceDescriptor);
}

const struct UsbInterface usbClcInterface =
{
	.endpoint = CLC_EP_ID,
	.reportLengthFor = &usbClcReportLengthFor,
	.getOutputReportBufferFor = &usbClcGetOutputReportBufferFor,
	.onReportReceived = &usbClcOnReportReceived,
	.getInputReportFor = NULL,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbClcMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbClcGetHidDescriptor,
	.getStringDescriptor = &usbClcGetStringDescriptor,
	.getReportDescriptor = &usbClcGetReportDescriptor
};
