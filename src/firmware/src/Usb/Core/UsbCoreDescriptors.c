#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbCoreInterface.h"

static const uint8_t usbCoreReportDescriptors[] =
{
	#include "CoreReportDescriptors.txt"
};

int16_t usbCoreGetReportDescriptor(const void **ptr)
{
	*ptr = usbCoreReportDescriptors;
	return sizeof(usbCoreReportDescriptors);
}

STRING_DESCRIPTOR(usbCoreInterfaceName, 'C', 'o', 'r', 'e');

static int16_t usbCoreGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbCoreInterfaceName;
		return *((uint8_t *) &usbCoreInterfaceName);
	}

	return -1;
}

static const struct UsbInOutInterface(1) usbCoreInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = CORE_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(CORE, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbCoreReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | CORE_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_CORE_IN_LEN,
		.bInterval = 1
	},
	.ep_out =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_OUT | CORE_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_CORE_OUT_LEN,
		.bInterval = 1
	}
};

static uint16_t usbCoreMemcpyInterfaceDescriptor(void *dest)
{
	memcpy(dest, &usbCoreInterfaceDescriptor, sizeof(usbCoreInterfaceDescriptor));
	return sizeof(usbCoreInterfaceDescriptor);
}

static int16_t usbCoreGetHidDescriptor(const void **ptr)
{
	*ptr = &usbCoreInterfaceDescriptor.hid;
	return sizeof(usbCoreInterfaceDescriptor);
}

const struct UsbInterface usbCoreInterface =
{
	.endpoint = CORE_EP_ID,
	.initialise = &usbCoreInitialise,
	.reportLengthFor = &usbCoreReportLengthFor,
	.getOutputReportBufferFor = &usbCoreGetOutputReportBufferFor,
	.onReportReceived = &usbCoreOnReportReceived,
	.getInputReportFor = &usbCoreGetInputReportFor,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbCoreMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbCoreGetHidDescriptor,
	.getStringDescriptor = &usbCoreGetStringDescriptor,
	.getReportDescriptor = &usbCoreGetReportDescriptor,
	.assignPin = &usbCoreAssignPin
};
