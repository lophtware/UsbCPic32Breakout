#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_BENCHTESTING_BENCHTESTING_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_BENCHTESTING_BENCHTESTING_H
#ifndef ENABLE_BENCH_TESTING
#define ENABLE_BENCH_TESTING
#endif

#include "../BenchTesting.h"

#define COMMAND_TOGGLE_ALL_PINS 0x01
#define COMMAND_TOGGLE_ALL_PINS_WITH_PWM 0x02

struct BenchTestingCommand
{
	union
	{
		uint32_t raw;

		struct
		{
			uint8_t type;
			uint8_t args[3];
		} fields;
	} as;
};

extern QueueHandle_t benchTestingCommands;

extern void benchTestingTask(void *args);
extern void benchTestingTaskToggleAllPinsForSeconds(uint16_t seconds);
extern void benchTestingTaskToggleAllPinsWithPwmForSeconds(uint16_t seconds, uint8_t period);

#endif
