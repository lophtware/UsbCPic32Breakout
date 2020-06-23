#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbSpiInterface.h"

static const uint8_t usbSpiReportDescriptors[] =
{
	#include "SpiReportDescriptors.txt"
};

int16_t usbSpiGetReportDescriptor(const void **ptr)
{
	*ptr = usbSpiReportDescriptors;
	return sizeof(usbSpiReportDescriptors);
}

STRING_DESCRIPTOR(usbSpiInterfaceName, 'S', 'P', 'I');

static int16_t usbSpiGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbSpiInterfaceName;
		return *((uint8_t *) &usbSpiInterfaceName);
	}

	return -1;
}

static const struct UsbInOutInterface(1) usbSpiInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = SPI_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(SPI, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbSpiReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | SPI_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_SPI_IN_LEN,
		.bInterval = 1
	},
	.ep_out =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_OUT | SPI_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_SPI_OUT_LEN,
		.bInterval = 1
	}
};

static uint16_t usbSpiMemcpyInterfaceDescriptor(void *dest)
{
#ifdef SPI_INTERFACE_ENABLED
	memcpy(dest, &usbSpiInterfaceDescriptor, sizeof(usbSpiInterfaceDescriptor));
	return sizeof(usbSpiInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbSpiGetHidDescriptor(const void **ptr)
{
	*ptr = &usbSpiInterfaceDescriptor.hid;
	return sizeof(usbSpiInterfaceDescriptor);
}

const struct UsbInterface usbSpiInterface =
{
	.endpoint = SPI_EP_ID,
	.initialise = &usbSpiInitialise,
	.reportLengthFor = &usbSpiReportLengthFor,
	.getOutputReportBufferFor = &usbSpiGetOutputReportBufferFor,
	.onReportReceived = &usbSpiOnReportReceived,
	.getInputReportFor = &usbSpiGetInputReportFor,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbSpiMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbSpiGetHidDescriptor,
	.getStringDescriptor = &usbSpiGetStringDescriptor,
	.getReportDescriptor = &usbSpiGetReportDescriptor,
	.assignPin = &usbSpiAssignPin,
	.unassignPin = &usbSpiUnassignPin
};
