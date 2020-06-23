#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbUartInterface.h"

static const uint8_t usbUartReportDescriptors[] =
{
	#include "UartReportDescriptors.txt"
};

int16_t usbUartGetReportDescriptor(const void **ptr)
{
	*ptr = usbUartReportDescriptors;
	return sizeof(usbUartReportDescriptors);
}

STRING_DESCRIPTOR(usbUartInterfaceName, 'U', 'A', 'R', 'T');

static int16_t usbUartGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbUartInterfaceName;
		return *((uint8_t *) &usbUartInterfaceName);
	}

	return -1;
}

static const struct UsbInOutInterface(1) usbUartInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = UART_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(UART, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbUartReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | UART_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_UART_IN_LEN,
		.bInterval = 1
	},
	.ep_out =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_OUT | UART_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_UART_OUT_LEN,
		.bInterval = 1
	}
};

static uint16_t usbUartMemcpyInterfaceDescriptor(void *dest)
{
#ifdef UART_INTERFACE_ENABLED
	memcpy(dest, &usbUartInterfaceDescriptor, sizeof(usbUartInterfaceDescriptor));
	return sizeof(usbUartInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbUartGetHidDescriptor(const void **ptr)
{
	*ptr = &usbUartInterfaceDescriptor.hid;
	return sizeof(usbUartInterfaceDescriptor);
}

const struct UsbInterface usbUartInterface =
{
	.endpoint = UART_EP_ID,
	.reportLengthFor = &usbUartReportLengthFor,
	.getOutputReportBufferFor = &usbUartGetOutputReportBufferFor,
	.onReportReceived = &usbUartOnReportReceived,
	.getInputReportFor = NULL,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbUartMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbUartGetHidDescriptor,
	.getStringDescriptor = &usbUartGetStringDescriptor,
	.getReportDescriptor = &usbUartGetReportDescriptor
};
