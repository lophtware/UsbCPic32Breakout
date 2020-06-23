#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_UNLOCKKEY_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_UNLOCKKEY_H
#include <stdint.h>
#include <stdbool.h>

struct UnlockKey
{
	union
	{
		uint8_t bytes[8];
		uint32_t dwords[2];
	} as;
};

extern void unlockKeySet(const struct UnlockKey *key);
extern bool unlockKeyMatches(const struct UnlockKey *key);

#endif
