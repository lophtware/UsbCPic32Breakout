#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "I2c.h"

#define PROTECT_ZERO_MASK 0x8000

static uint8_t i2cRam[1024];
static uint16_t i2cRamProtectedAddressMask;
static uint16_t i2cRamWriteProtectFlag;

void i2cRamInitialise(void)
{
	i2cRamProtectedAddressMask = 0x03ff | PROTECT_ZERO_MASK;
	i2cRamWriteProtectFlag = true;
	memcpy(i2cRam, (const void *) i2cNvmPage.data.ramInitialisation, sizeof(i2cNvmPage.data.ramInitialisation));
}

void i2cRamSetProtectedAddressMask(uint16_t mask)
{
	i2cRamProtectedAddressMask = mask & (0x03ff | PROTECT_ZERO_MASK);
}

uint16_t i2cRamGetProtectedAddressMask(void)
{
	return i2cRamProtectedAddressMask;
}

void i2cRamEnableWriteProtection(void)
{
	i2cRamWriteProtectFlag = true;
}

void i2cRamDisableWriteProtection(void)
{
	i2cRamWriteProtectFlag = false;
}

bool i2cRamIsWriteProtected(void)
{
	return i2cRamWriteProtectFlag;
}

uint8_t i2cRamReadByte(uint16_t address)
{
	if (address >= sizeof(i2cRam))
		return 0x00;

	return i2cRam[address];
}

bool i2cRamWriteByte(uint16_t address, uint8_t value)
{
	if (
		i2cRamWriteProtectFlag ||
		address >= sizeof(i2cRam) ||
		(address & i2cRamProtectedAddressMask) ||
		(address == 0x000 && (i2cRamProtectedAddressMask & PROTECT_ZERO_MASK)))
	{
		return false;
	}

	i2cRam[address] = value;
	return true;
}

void i2cRamReadBytesUnprotected(uint8_t *buffer, uint16_t address, uint16_t count)
{
	if (count == 0)
		return;

	if (count > sizeof(i2cRam))
		count = sizeof(i2cRam);

	address &= sizeof(i2cRam) - 1;

	if ((address + count) > sizeof(i2cRam))
	{
		uint16_t firstBlock = sizeof(i2cRam) - address;
		memcpy(buffer, &i2cRam[address], firstBlock);
		memcpy(buffer + firstBlock, &i2cRam[0], count - firstBlock);
	}
	else
		memcpy(buffer, &i2cRam[address], count);
}

bool i2cRamWriteBytesUnprotected(uint16_t address, const uint8_t *contents, uint16_t count)
{
	if (count == 0)
		return true;

	if (count > sizeof(i2cRam))
		count = sizeof(i2cRam);

	address &= sizeof(i2cRam) - 1;

	if ((address + count) > sizeof(i2cRam))
	{
		uint16_t firstBlock = sizeof(i2cRam) - address;
		memcpy(&i2cRam[address], contents, firstBlock);
		memcpy(&i2cRam[0], &contents[firstBlock], count - firstBlock);
	}
	else
		memcpy(&i2cRam[address], contents, count);

	return true;
}

bool i2cRamInitialisationStoreAndReset(const uint8_t *contents, uint16_t offset, uint16_t count)
{
	if (!contents || (offset + count) > I2C_RAM_SIZE_BYTES || count == 0)
		return false;

	return i2cNvmStoreAndReset(
		contents,
		(uint16_t) (&i2cNvmPage.data.ramInitialisation[offset] - &i2cNvmPage.data.ramInitialisation[0]),
		count);
}

volatile const uint8_t *i2cRamInitialisationGetContents(void)
{
	return i2cNvmPage.data.ramInitialisation;
}
