#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "Fault.h"
#include "Version.h"
#include "DeviceInfoI2cBank.h"
#include "I2c.h"

#define MAGIC_WORD_0 0x7d90b0fb
#define MAGIC_WORD_1 0x744709dd
#define MAGIC_WORD_2 0x717f318c
#define MAGIC_WORD_3 0x2cb912f9

uint8_t deviceInfoI2cBankReadByte(uint16_t address)
{
	switch (address)
	{
		I2C_BANK_WORD_REGISTER(0x0000, MAGIC_WORD_0);
		I2C_BANK_WORD_REGISTER(0x0004, MAGIC_WORD_1);
		I2C_BANK_WORD_REGISTER(0x0008, MAGIC_WORD_2);
		I2C_BANK_WORD_REGISTER(0x000c, MAGIC_WORD_3);

		I2C_BANK_WORD_REGISTER(0x0010, VERSION_FIRMWARE_AS_WORD);
		I2C_BANK_WORD_REGISTER(0x0014, VERSION_USB_API_AS_WORD);

		I2C_BANK_WORD_REGISTER(0x0018, DEVID);
		I2C_BANK_WORD_REGISTER(0x001c, UDID1);
		I2C_BANK_WORD_REGISTER(0x0020, UDID2);
		I2C_BANK_WORD_REGISTER(0x0024, UDID3);
		I2C_BANK_WORD_REGISTER(0x0028, UDID4);
		I2C_BANK_WORD_REGISTER(0x002c, UDID5);

		I2C_BANK_WORD_REGISTER(0x0040, faultGetResetReason());
		I2C_BANK_WORD_REGISTER(0x0044, faultGetExceptionDetailErrorEpc());
		I2C_BANK_WORD_REGISTER(0x0048, faultGetExceptionDetailEpc());
		I2C_BANK_WORD_REGISTER(0x004c, faultGetExceptionDetailCause());
		I2C_BANK_WORD_REGISTER(0x0050, faultGetExceptionDetailDesave());
		I2C_BANK_WORD_REGISTER(0x0054, faultGetExceptionDetailRa());
		I2C_BANK_WORD_REGISTER(0x0058, (uint32_t) (faultGetDebugDword() & 0xfffffffful));
		I2C_BANK_WORD_REGISTER(0x005c, (uint32_t) ((faultGetDebugDword() >> 32) & 0xfffffffful));

		default:
			return 0x00;
	}
}
