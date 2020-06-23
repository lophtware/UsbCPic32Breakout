#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbCcpInterface.h"

static const uint8_t usbCcpReportDescriptors[] =
{
	#include "CcpReportDescriptors.txt"
};

int16_t usbCcpGetReportDescriptor(const void **ptr)
{
	*ptr = usbCcpReportDescriptors;
	return sizeof(usbCcpReportDescriptors);
}

STRING_DESCRIPTOR(usbCcpInterfaceName, 'C', 'C', 'P');

static int16_t usbCcpGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbCcpInterfaceName;
		return *((uint8_t *) &usbCcpInterfaceName);
	}

	return -1;
}

static const struct UsbInInterface(1) usbCcpInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = CCP_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(CCP, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbCcpReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | CCP_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_CCP_IN_LEN,
		.bInterval = 1
	}
};

static uint16_t usbCcpMemcpyInterfaceDescriptor(void *dest)
{
#ifdef CCP_INTERFACE_ENABLED
	memcpy(dest, &usbCcpInterfaceDescriptor, sizeof(usbCcpInterfaceDescriptor));
	return sizeof(usbCcpInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbCcpGetHidDescriptor(const void **ptr)
{
	*ptr = &usbCcpInterfaceDescriptor.hid;
	return sizeof(usbCcpInterfaceDescriptor);
}

const struct UsbInterface usbCcpInterface =
{
	.endpoint = CCP_EP_ID,
	.reportLengthFor = &usbCcpReportLengthFor,
	.getOutputReportBufferFor = &usbCcpGetOutputReportBufferFor,
	.onReportReceived = &usbCcpOnReportReceived,
	.getInputReportFor = NULL,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbCcpMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbCcpGetHidDescriptor,
	.getStringDescriptor = &usbCcpGetStringDescriptor,
	.getReportDescriptor = &usbCcpGetReportDescriptor
};
