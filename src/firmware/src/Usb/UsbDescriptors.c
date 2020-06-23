#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../Version.h"

#include "Usb.h"

#include "usb_config.h"
#include "m-stack/include/usb.h"
#include "m-stack/include/usb_ch9.h"
#include "m-stack/include/usb_hid.h"

const struct device_descriptor usbDeviceDescriptor =
{
	.bLength = sizeof(struct device_descriptor),
	.bDescriptorType = DESC_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubclass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = EP_0_LEN,
	.idVendor = 0x1209,
	.idProduct = 0xcb0b,
	.bcdDevice = (VERSION_USB_API_MAJOR_BCD << 8) | VERSION_USB_API_MINOR_BCD,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = NUMBER_OF_CONFIGURATIONS
};

STRING_DESCRIPTOR(usbSupportedLanguages, LANGID_UK);
STRING_DESCRIPTOR(usbVendor, 'l', 'o', 'p', 'h', 't', 'w', 'a', 'r', 'e');
STRING_DESCRIPTOR(usbProduct, 'U', 'S', 'B', '2', '.', '0', ' ', 'F', 'u', 'l', 'l', '-', 'S', 'p', 'e', 'e', 'd', ' ', 'T', 'y', 'p', 'e', '-', 'C', ' ', '/', ' ', 'P', 'I', 'C', '3', '2', ' ', 'B', 'r', 'e', 'a', 'k', 'o', 'u', 't', ' ', 'B', 'o', 'a', 'r', 'd');
NON_CONST_STRING_DESCRIPTOR(usbSerialNumber, '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0');
NON_CONST_STRING_DESCRIPTOR(usbConfiguration1Name, 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', ' ', '1', '\0');
NON_CONST_STRING_DESCRIPTOR(usbConfiguration2Name, 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', ' ', '2', '\0');
NON_CONST_STRING_DESCRIPTOR(usbConfiguration3Name, 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', ' ', '3', '\0');
NON_CONST_STRING_DESCRIPTOR(usbConfiguration4Name, 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', ' ', '4', '\0');

static const void *const usbStringDescriptors[] =
{
	&usbSupportedLanguages, &usbVendor, &usbProduct, &usbSerialNumber,
	&usbConfiguration1Name, &usbConfiguration2Name, &usbConfiguration3Name, &usbConfiguration4Name
};

void usbDescriptorsInitialise(void)
{
	static const uint8_t hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	const uint8_t *serial = (const uint8_t *) &UDID1;
	uint16_t *descriptor = usbSerialNumber.chars;
	for (int i = 0; i < usbSerialNumber.bLength / 4; i++)
	{
		*(descriptor++) = hex[(*serial >> 4) & 0x0f];
		*(descriptor++) = hex[(*serial >> 0) & 0x0f];
		serial++;
	}
}

int16_t usbOnGetStringDescriptor(uint8_t index, const void **ptr)
{
	uint8_t interfacePlusOne = (index >> 4) & 0x0f;
	if (interfacePlusOne == 0)
	{
		if (index < sizeof(usbStringDescriptors) / sizeof(const void *const))
		{
			*ptr = usbStringDescriptors[index];
			return *((uint8_t *) usbStringDescriptors[index]);
		}

		return -1;
	}

	if (interfacePlusOne > NUMBER_OF_INTERFACES)
		return -1;

	return usbInterfaces[interfacePlusOne - 1]->getStringDescriptor(index & 0x0f, ptr);
}

bool usbDescriptorsSetConfigurationName(uint8_t descriptorIndex, const uint8_t *unicodeName)
{
	if (descriptorIndex >= sizeof(usbStringDescriptors) / sizeof(const void *const))
		return false;

	uint8_t length = sizeof(usbConfiguration1Name.chars) / 2;
	for (uint8_t i = 0; i < length * 2; i += 2)
	{
		if ((((uint16_t) unicodeName[i + 1] << 8) | unicodeName[i]) == 0)
		{
			length = i / 2;
			break;
		}
	}

	if (length == 0)
		return false;

	if (usbStringDescriptors[descriptorIndex] == &usbConfiguration1Name)
	{
		memcpy(usbConfiguration1Name.chars, unicodeName, sizeof(usbConfiguration1Name.chars));
		usbConfiguration1Name.bLength = length;
	}
	else if (usbStringDescriptors[descriptorIndex] == &usbConfiguration2Name)
	{
		memcpy(usbConfiguration2Name.chars, unicodeName, sizeof(usbConfiguration2Name.chars));
		usbConfiguration2Name.bLength = length;
	}
	else if (usbStringDescriptors[descriptorIndex] == &usbConfiguration3Name)
	{
		memcpy(usbConfiguration3Name.chars, unicodeName, sizeof(usbConfiguration3Name.chars));
		usbConfiguration3Name.bLength = length;
	}
	else if (usbStringDescriptors[descriptorIndex] == &usbConfiguration4Name)
	{
		memcpy(usbConfiguration4Name.chars, unicodeName, sizeof(usbConfiguration4Name.chars));
		usbConfiguration4Name.bLength = length;
	}
	else
		return false;

	return true;
}

STATIC_SIZE_CHECK_EQUAL(USB_ARRAYLEN(USB_CONFIG_DESCRIPTOR_MAP), NUMBER_OF_CONFIGURATIONS);
STATIC_SIZE_CHECK_EQUAL(sizeof(USB_DEVICE_DESCRIPTOR), 18);
