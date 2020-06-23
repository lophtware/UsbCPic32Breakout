#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbTimerInterface.h"

static const uint8_t usbTimerReportDescriptors[] =
{
	#include "TimerReportDescriptors.txt"
};

int16_t usbTimerGetReportDescriptor(const void **ptr)
{
	*ptr = usbTimerReportDescriptors;
	return sizeof(usbTimerReportDescriptors);
}

STRING_DESCRIPTOR(usbTimerInterfaceName, 'T', 'i', 'm', 'e', 'r');

static int16_t usbTimerGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbTimerInterfaceName;
		return *((uint8_t *) &usbTimerInterfaceName);
	}

	return -1;
}

static const struct UsbInInterface(1) usbTimerInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = TIMER_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(TIMER, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbTimerReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | TIMER_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_TIMER_IN_LEN,
		.bInterval = 1
	}
};

static uint16_t usbTimerMemcpyInterfaceDescriptor(void *dest)
{
#ifdef TIMER_INTERFACE_ENABLED
	memcpy(dest, &usbTimerInterfaceDescriptor, sizeof(usbTimerInterfaceDescriptor));
	return sizeof(usbTimerInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbTimerGetHidDescriptor(const void **ptr)
{
	*ptr = &usbTimerInterfaceDescriptor.hid;
	return sizeof(usbTimerInterfaceDescriptor);
}

const struct UsbInterface usbTimerInterface =
{
	.endpoint = TIMER_EP_ID,
	.reportLengthFor = &usbTimerReportLengthFor,
	.getOutputReportBufferFor = &usbTimerGetOutputReportBufferFor,
	.onReportReceived = &usbTimerOnReportReceived,
	.getInputReportFor = NULL,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbTimerMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbTimerGetHidDescriptor,
	.getStringDescriptor = &usbTimerGetStringDescriptor,
	.getReportDescriptor = &usbTimerGetReportDescriptor
};
