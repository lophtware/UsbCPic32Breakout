#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../FreeRtos.h"
#include "../../I2c.h"

#include "UsbI2cInterface.h"

#define ADDRESS_RAM_IS_WRITE_PROTECTED 0x0010
#define ADDRESS_RAM_PROTECTED_MASK (ADDRESS_RAM_IS_WRITE_PROTECTED + 1)

#define ADDRESS_ROM_IS_WRITE_PROTECTED 0x0020
#define ADDRESS_ROM_PROTECTED_MASK (ADDRESS_ROM_IS_WRITE_PROTECTED + 1)

uint8_t usbI2cBankReadByte(uint16_t address)
{
	switch (address)
	{
		I2C_BANK_BYTE_REGISTER(ADDRESS_RAM_IS_WRITE_PROTECTED, i2cRamIsWriteProtected() ? 0x01 : 0x00);
		I2C_BANK_HALF_WORD_REGISTER(ADDRESS_RAM_PROTECTED_MASK, i2cRamGetProtectedAddressMask());

		I2C_BANK_BYTE_REGISTER(ADDRESS_ROM_IS_WRITE_PROTECTED, i2cRomIsWriteProtected() ? 0x01 : 0x00);
		I2C_BANK_HALF_WORD_REGISTER(ADDRESS_ROM_PROTECTED_MASK, i2cRomGetProtectedAddressMask());

		default:
			return 0x00;
	}
}

bool usbI2cBankWriteByte(uint16_t address, uint8_t value)
{
	switch (address)
	{
		case ADDRESS_RAM_IS_WRITE_PROTECTED:
			if (value)
				i2cRamEnableWriteProtection();
			else
				i2cRamDisableWriteProtection();

			return true;

		case ADDRESS_ROM_IS_WRITE_PROTECTED:
			if (value)
				i2cRomEnableWriteProtection();
			else
				i2cRomDisableWriteProtection();

			return true;
			
		default:
			return false;
	}
}
