#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbComparatorInterface.h"

static const uint8_t usbComparatorReportDescriptors[] =
{
	#include "ComparatorReportDescriptors.txt"
};

int16_t usbComparatorGetReportDescriptor(const void **ptr)
{
	*ptr = usbComparatorReportDescriptors;
	return sizeof(usbComparatorReportDescriptors);
}

STRING_DESCRIPTOR(usbComparatorInterfaceName, 'C', 'o', 'm', 'p', 'a', 'r', 'a', 't', 'o', 'r');

static int16_t usbComparatorGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbComparatorInterfaceName;
		return *((uint8_t *) &usbComparatorInterfaceName);
	}

	return -1;
}

static const struct UsbInInterface(1) usbComparatorInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = COMPARATOR_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(COMPARATOR, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbComparatorReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | COMPARATOR_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_COMPARATOR_IN_LEN,
		.bInterval = 1
	}
};

static uint16_t usbComparatorMemcpyInterfaceDescriptor(void *dest)
{
#ifdef COMPARATOR_INTERFACE_ENABLED
	memcpy(dest, &usbComparatorInterfaceDescriptor, sizeof(usbComparatorInterfaceDescriptor));
	return sizeof(usbComparatorInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbComparatorGetHidDescriptor(const void **ptr)
{
	*ptr = &usbComparatorInterfaceDescriptor.hid;
	return sizeof(usbComparatorInterfaceDescriptor);
}

const struct UsbInterface usbComparatorInterface =
{
	.endpoint = COMPARATOR_EP_ID,
	.reportLengthFor = &usbComparatorReportLengthFor,
	.getOutputReportBufferFor = &usbComparatorGetOutputReportBufferFor,
	.onReportReceived = &usbComparatorOnReportReceived,
	.getInputReportFor = NULL,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbComparatorMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbComparatorGetHidDescriptor,
	.getStringDescriptor = &usbComparatorGetStringDescriptor,
	.getReportDescriptor = &usbComparatorGetReportDescriptor
};
