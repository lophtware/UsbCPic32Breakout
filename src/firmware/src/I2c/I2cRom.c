#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../Fault.h"
#include "../Nvm.h"

#include "I2c.h"

#define PROTECT_ZERO_MASK 0x8000
#define UNINITIALISED_ADDRESS 0xffff

static struct
{
	union
	{
		uint8_t asBytes[8];
		uint64_t asDword;
	} data;

	uint16_t address;
} i2cRomBuffer;

static uint16_t i2cRomProtectedAddressMask;
static bool i2cRomWriteProtectFlag;

void i2cRomInitialise(void)
{
	i2cRomProtectedAddressMask = 0x03ff | PROTECT_ZERO_MASK;
	i2cRomWriteProtectFlag = true;
	i2cRomBuffer.address = UNINITIALISED_ADDRESS;
}

void i2cRomSetProtectedAddressMask(uint16_t mask)
{
	i2cRomProtectedAddressMask = mask & (0x03ff | PROTECT_ZERO_MASK);
}

uint16_t i2cRomGetProtectedAddressMask(void)
{
	return i2cRomProtectedAddressMask;
}

void i2cRomEnableWriteProtection(void)
{
	i2cRomWriteProtectFlag = true;
}

void i2cRomDisableWriteProtection(void)
{
	i2cRomWriteProtectFlag = false;
}

bool i2cRomIsWriteProtected(void)
{
	return i2cRomWriteProtectFlag;
}

uint8_t i2cRomReadByte(uint16_t address)
{
	if (address >= sizeof(i2cNvmPage.data.rom))
		return 0x00;

	return i2cNvmPage.data.rom.asBytes[address];
}

bool i2cRomWriteByte(uint16_t address, uint8_t value)
{
	if (
		i2cRomWriteProtectFlag ||
		address >= sizeof(i2cNvmPage.data.rom) ||
		(address & i2cRomProtectedAddressMask) ||
		(address == 0x000 && (i2cRomProtectedAddressMask & PROTECT_ZERO_MASK)))
	{
		return false;
	}

	uint32_t dwordAddress = (address >> 3) & 0x1fff;
	if (dwordAddress != i2cRomBuffer.address)
	{
		if (i2cRomBuffer.address < sizeof(i2cNvmPage.data.rom) / 8)
			nvmWriteDword(&i2cNvmPage.data.rom.asDwords[i2cRomBuffer.address], i2cRomBuffer.data.asDword);

		i2cRomBuffer.address = dwordAddress;
		i2cRomBuffer.data.asDword = i2cNvmPage.data.rom.asDwords[i2cRomBuffer.address];
	}

	i2cRomBuffer.data.asBytes[address & 7] = value;
	return true;
}

void i2cRomReadBytesUnprotected(uint8_t *buffer, uint16_t address, uint16_t count)
{
	if (count == 0)
		return;

	if (count > sizeof(i2cNvmPage.data.rom))
		count = sizeof(i2cNvmPage.data.rom);

	address &= sizeof(i2cNvmPage.data.rom) - 1;

	if ((address + count) > sizeof(i2cNvmPage.data.rom))
	{
		uint16_t firstBlock = sizeof(i2cNvmPage.data.rom) - address;
		memcpy(&buffer[0], (const void *) &i2cNvmPage.data.rom.asBytes[address], firstBlock);
		memcpy(&buffer[firstBlock], (const void *) &i2cNvmPage.data.rom.asBytes[0], count - firstBlock);
	}
	else
		memcpy(buffer, (const void *) &i2cNvmPage.data.rom.asBytes[address], count);
}

bool i2cRomWriteBytesUnprotected(uint16_t address, const uint8_t *contents, uint16_t count)
{
	if (count == 0)
		return true;

	if (count > sizeof(i2cNvmPage.data.rom))
		count = sizeof(i2cNvmPage.data.rom);

	address &= sizeof(i2cNvmPage.data.rom) - 1;

	bool result = true;
	uint64_t dword = i2cNvmPage.data.rom.asDwords[address >> 3];
	uint16_t dwordOffset = address & 7;
	for (uint16_t i = 0; i < count; i++)
	{
		((uint8_t *) &dword)[dwordOffset] = contents[i];
		if (dwordOffset == 7 || (i + 1) == count)
		{
			result = result && nvmWriteDword(&i2cNvmPage.data.rom.asDwords[address >> 3], dword);
			address = (address & ~7) + 8;
			if (address >= sizeof(i2cNvmPage.data.rom))
				address = 0;

			dword = i2cNvmPage.data.rom.asDwords[address >> 3];
			dwordOffset = 0;
		}
		else
			dwordOffset++;
	}

	return result;
}

void i2cRomOnTransactionDone(void)
{
	if (i2cRomBuffer.address >= sizeof(i2cNvmPage.data.rom) / 8)
		return;

	nvmWriteDword(&i2cNvmPage.data.rom.asDwords[i2cRomBuffer.address], i2cRomBuffer.data.asDword);
	i2cRomBuffer.address = UNINITIALISED_ADDRESS;
}

bool i2cRomStoreAndReset(const uint8_t *contents, uint16_t offset, uint16_t count)
{
	if (!contents || (offset + count) > I2C_ROM_SIZE_BYTES || count == 0)
		return false;

	return i2cNvmStoreAndReset(
		contents,
		(uint16_t) (&i2cNvmPage.data.rom.asBytes[offset] - &i2cNvmPage.data.rom.asBytes[0]),
		count);
}

bool i2cNvmStoreAndReset(const uint8_t *contents, uint16_t offset, uint16_t count)
{
	if (!contents || (offset + count) > sizeof(i2cNvmPage.page) || count == 0)
		return false;

	__builtin_disable_interrupts();
	static const uint8_t *staticContents;
	static uint16_t staticOffset;
	static uint16_t staticCount;
	staticContents = contents;
	staticOffset = offset;
	staticCount = count;

	/*** THIS FUNCTION'S STACK PROBABLY RESIDES INSIDE THE FREERTOS HEAP, WHICH IS ABOUT TO BE HORRIBLY MANGLED...AVOID LOCAL VARIABLES AFTER THIS !!! ***/
	RESET_STACK_POINTER_TO_TOP();

	static uint8_t *heap;
	heap = (uint8_t *) getHeapSection();
	static union I2cNvmPage *nvmInHeap;
	nvmInHeap = (union I2cNvmPage *) ALIGNED(heap, 8);

	memcpy(nvmInHeap, (const void *) &i2cNvmPage, sizeof(i2cNvmPage.page));
	memcpy(&nvmInHeap->page[staticOffset], staticContents, staticCount);

	if (nvmErasePages2048(&i2cNvmPage, 1) != 1)
		faultReset(RESET_REASON_INVALID_CONFIGURATION(0x20));

	static const uint8_t *nvmPtr;
	static const uint8_t *tooFar;
	static uint8_t *heapPtr;
	nvmPtr = (const uint8_t *) &i2cNvmPage;
	tooFar = (const uint8_t *) (&i2cNvmPage + 1);
	heapPtr = (uint8_t *) nvmInHeap;
	while (nvmPtr < tooFar)
	{
		nvmWriteRow256(nvmPtr, heapPtr);
		nvmPtr += 256;
		heapPtr += 256;
	}

	faultReset(RESET_REASON_I2C_NVM_UPDATED);
	return true;
}

volatile const uint8_t *i2cRomGetContents(void)
{
	return i2cNvmPage.data.rom.asBytes;
}
