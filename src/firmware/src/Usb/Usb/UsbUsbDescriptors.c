#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbUsbInterface.h"

static const uint8_t usbUsbReportDescriptors[] =
{
	#include "UsbReportDescriptors.txt"
};

int16_t usbUsbGetReportDescriptor(const void **ptr)
{
	*ptr = usbUsbReportDescriptors;
	return sizeof(usbUsbReportDescriptors);
}

STRING_DESCRIPTOR(usbUsbInterfaceName, 'U', 'S', 'B');

static int16_t usbUsbGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbUsbInterfaceName;
		return *((uint8_t *) &usbUsbInterfaceName);
	}

	return -1;
}

static const struct UsbInOutInterface(1) usbUsbInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = USB_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(USB, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbUsbReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | USB_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_USB_IN_LEN,
		.bInterval = 10
	},
	.ep_out =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_OUT | USB_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_USB_OUT_LEN,
		.bInterval = 10
	}
};

static uint16_t usbUsbMemcpyInterfaceDescriptor(void *dest)
{
	memcpy(dest, &usbUsbInterfaceDescriptor, sizeof(usbUsbInterfaceDescriptor));
	return sizeof(usbUsbInterfaceDescriptor);
}

static int16_t usbUsbGetHidDescriptor(const void **ptr)
{
	*ptr = &usbUsbInterfaceDescriptor.hid;
	return sizeof(usbUsbInterfaceDescriptor);
}

const struct UsbInterface usbUsbInterface =
{
	.endpoint = USB_EP_ID,
	.initialise = &usbUsbInitialise,
	.reportLengthFor = &usbUsbReportLengthFor,
	.getOutputReportBufferFor = &usbUsbGetOutputReportBufferFor,
	.onReportReceived = &usbUsbOnReportReceived,
	.getInputReportFor = &usbUsbGetInputReportFor,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbUsbMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbUsbGetHidDescriptor,
	.getStringDescriptor = &usbUsbGetStringDescriptor,
	.getReportDescriptor = &usbUsbGetReportDescriptor,
	.assignPin = &usbUsbAssignPin,
	.unassignPin = &usbUsbUnassignPin
};
