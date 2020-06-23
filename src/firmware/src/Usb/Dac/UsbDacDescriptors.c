#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "UsbDacInterface.h"

static const uint8_t usbDacReportDescriptors[] =
{
	#include "DacReportDescriptors.txt"
};

int16_t usbDacGetReportDescriptor(const void **ptr)
{
	*ptr = usbDacReportDescriptors;
	return sizeof(usbDacReportDescriptors);
}

STRING_DESCRIPTOR(usbDacInterfaceName, 'D', 'A', 'C');

static int16_t usbDacGetStringDescriptor(uint8_t index, const void **ptr)
{
	if (index == 0)
	{
		*ptr = &usbDacInterfaceName;
		return *((uint8_t *) &usbDacInterfaceName);
	}

	return -1;
}

static const struct UsbInterface(1) usbDacInterfaceDescriptor =
{
	.interface =
	{
		.bLength = sizeof(struct interface_descriptor),
		.bDescriptorType = DESC_INTERFACE,
		.bInterfaceNumber = DAC_INTERFACE_ID,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = HID_INTERFACE_CLASS,
		.bInterfaceSubclass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = INDEX_OF_INTERFACE_DESCRIPTOR(DAC, 0)
	},
	.hid =
	{
		.bLength = sizeof(struct hid_descriptor),
		.bDescriptorType = DESC_HID,
		.bcdHID = HID_VERSION,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType2 = DESC_REPORT,
		.wDescriptorLength = sizeof(usbDacReportDescriptors)
	}
};

static uint16_t usbDacMemcpyInterfaceDescriptor(void *dest)
{
#ifdef DAC_INTERFACE_ENABLED
	memcpy(dest, &usbDacInterfaceDescriptor, sizeof(usbDacInterfaceDescriptor));
	return sizeof(usbDacInterfaceDescriptor);
#else
	return 0;
#endif
}

static int16_t usbDacGetHidDescriptor(const void **ptr)
{
	*ptr = &usbDacInterfaceDescriptor.hid;
	return sizeof(usbDacInterfaceDescriptor);
}

const struct UsbInterface usbDacInterface =
{
	.endpoint = DAC_EP_ID,
	.reportLengthFor = &usbDacReportLengthFor,
	.getOutputReportBufferFor = &usbDacGetOutputReportBufferFor,
	.onReportReceived = &usbDacOnReportReceived,
	.getInputReportFor = NULL,
	.onReportSent = NULL,
	.memcpyInterfaceDescriptor = &usbDacMemcpyInterfaceDescriptor,
	.getHidDescriptor = &usbDacGetHidDescriptor,
	.getStringDescriptor = &usbDacGetStringDescriptor,
	.getReportDescriptor = &usbDacGetReportDescriptor
};
