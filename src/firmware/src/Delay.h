#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_DELAY_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_DELAY_H
#include <xc.h>
#include <stdint.h>

#define TICKS_PER_SECOND 12000000
#define TICKS_PER_MICROSECOND (TICKS_PER_SECOND / 1000000)

static inline uint32_t now(void)
{
	return _CP0_GET_COUNT();
}

static inline uint32_t elapsedSince(uint32_t startTime)
{
	return now() - startTime;
}

static inline void delayAtLeastMicroseconds(uint8_t us)
{
	uint32_t startTime = now();
	uint32_t usAsTicks = us * TICKS_PER_MICROSECOND;
	while (elapsedSince(startTime) <= usAsTicks)
		;;
}

#endif
