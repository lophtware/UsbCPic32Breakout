#include <xc.h>
#include <stdint.h>

#include "Crc32.h"

void crc32CalculateTable(uint32_t *table)
{
	if (!table)
		return;

	table[0] = 0;
	uint32_t crc32 = 1;
	for (uint32_t i = 128; i != 0; i >>= 1)
	{
		if (crc32 & 1)
			crc32 = (crc32 >> 1) ^ 0x82f63b78;
		else
			crc32 >>= 1;

		for (uint32_t j = 0; j < 256; j += i << 1)
			table[i + j] = crc32 ^ table[j];
	}
}

uint32_t crc32Calculate(const uint32_t *table, volatile const void *start, uint32_t length)
{
	if (!table)
		return 0;

	uint32_t crc32 = 0xfffffffful;
	volatile const uint8_t *ptr = (volatile const uint8_t *) start;
	volatile const uint8_t *tooFar = ptr + length;
	while (ptr < tooFar)
	{
		uint8_t tableIndex = (uint8_t) ((crc32 ^ *ptr) & 0xff);
		crc32 = (crc32 >> 8) ^ table[tableIndex];
		ptr++;
	}

	return crc32 ^ 0xfffffffful;
}
