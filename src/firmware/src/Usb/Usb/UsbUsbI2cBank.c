#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../FreeRtos.h"
#include "../../I2c.h"

#include "UsbUsbInterface.h"

#define ADDRESS_STATUS_FLAGS 0x010
#define ADDRESS_CURRENT_LIMIT (ADDRESS_STATUS_FLAGS + 4)
#define ADDRESS_CONFIGURATION_NUMBER (ADDRESS_CURRENT_LIMIT + 2)

#define ADDRESS_SEND_TO_HOST 0x200

uint8_t usbUsbI2cBankReadByte(uint16_t address)
{
	switch (address)
	{
		I2C_BANK_WORD_REGISTER(ADDRESS_STATUS_FLAGS, usbUsbGetStatusFlags());
		I2C_BANK_HALF_WORD_REGISTER(ADDRESS_CURRENT_LIMIT, usbUsbGetCurrentLimitMilliamps());
		I2C_BANK_BYTE_REGISTER(ADDRESS_CONFIGURATION_NUMBER, usb_get_configuration());

		default:
			return 0x00;
	}
}

bool usbUsbI2cBankWriteByte(uint16_t address, uint8_t value)
{
	static struct UsbSmallReport messageToHost =
	{
		.as =
		{
			.fields =
			{
				.payload = {USB_I2C_SEND_TO_HOST_REPORT_ID},
				.flags = { .endpoint = USB_EP_ID, .count = 7 }
			}
		}
	};

	switch (address)
	{
		case ADDRESS_SEND_TO_HOST:
			messageToHost.as.fields.payload[1 + 0] = value;
			return true;

		case ADDRESS_SEND_TO_HOST + 1:
			messageToHost.as.fields.payload[1 + 1] = value;
			return true;

		case ADDRESS_SEND_TO_HOST + 2:
			messageToHost.as.fields.payload[1 + 2] = value;
			return true;

		case ADDRESS_SEND_TO_HOST + 3:
			messageToHost.as.fields.payload[1 + 3] = value;
			return true;

		case ADDRESS_SEND_TO_HOST + 4:
			messageToHost.as.fields.payload[1 + 4] = value;
			return true;

		case ADDRESS_SEND_TO_HOST + 5:
			messageToHost.as.fields.payload[1 + 5] = value;
			return true;

		case ADDRESS_SEND_TO_HOST + 6:
			if (value == 3)
			{
				while (!usbSmallReportSend(messageToHost))
				{
					vTaskDelay(1);
				}

				return true;
			}
			else if (value == 1)
				return usbSmallReportSend(messageToHost);

			return false;

		default:
			return false;
	}
}
