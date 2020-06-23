#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbI2cInterface.h"

static const uint8_t usbI2cReportDescriptors[] =
{
	#include "I2cReportDescriptors.txt"
};

int16_t usbI2cGetReportDescriptor(const void **ptr)
{
	*ptr = usbI2cReportDescriptors;
	return sizeof(usbI2cReportDescriptors);
}

STRING_DESCRIPTOR(usbI2cInterfaceName, 'I', '2', 'C');

static int16_t usbI2cGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbI2cInterfaceName;
		return *((uint8_t *) &usbI2cInterfaceName);
	}

	return -1;
}

static const struct UsbInOutInterface(1) usbI2cInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = I2C_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(I2C, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbI2cReportDescriptors)
	},
	.ep_in =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_IN | I2C_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_I2C_IN_LEN,
		.bInterval = 1
	},
	.ep_out =
	{
		.bLength = sizeof(struct endpoint_descriptor),
		.bDescriptorType = DESC_ENDPOINT,
		.bEndpointAddress = EP_OUT | I2C_EP_ID,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = EP_I2C_OUT_LEN,
		.bInterval = 1
	}
};

static uint16_t usbI2cMemcpyInterfaceDescriptor(void *dest)
{
#ifdef I2C_INTERFACE_ENABLED
	memcpy(dest, &usbI2cInterfaceDescriptor, sizeof(usbI2cInterfaceDescriptor));
	return sizeof(usbI2cInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbI2cGetHidDescriptor(const void **ptr)
{
	*ptr = &usbI2cInterfaceDescriptor.hid;
	return sizeof(usbI2cInterfaceDescriptor);
}

const struct UsbInterface usbI2cInterface =
{
	.endpoint = I2C_EP_ID,
	.initialise = &usbI2cInitialise,
	.reportLengthFor = &usbI2cReportLengthFor,
	.getOutputReportBufferFor = &usbI2cGetOutputReportBufferFor,
	.onReportReceived = &usbI2cOnReportReceived,
	.getInputReportFor = &usbI2cGetInputReportFor,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbI2cMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbI2cGetHidDescriptor,
	.getStringDescriptor = &usbI2cGetStringDescriptor,
	.getReportDescriptor = &usbI2cGetReportDescriptor,
	.assignPin = &usbI2cAssignPin,
	.unassignPin = &usbI2cUnassignPin
};
