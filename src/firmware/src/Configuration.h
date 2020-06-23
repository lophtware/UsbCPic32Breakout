#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_CONFIGURATION_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_CONFIGURATION_H
#include <stdint.h>
#include <stdbool.h>
#include "UnlockKey.h"
#include "Pins.h"
#include "I2c.h"
#include "Spi.h"

struct SuspendedPinStates
{
	struct PinBankState bankStates[3];
};

struct PinAssignment
{
	uint8_t pin;
	uint8_t suspendBehaviour;
	uint8_t interface;
	uint64_t args;
};

struct PinAssignments
{
	struct PinAssignment map[PINS_NUMBER_CONFIGURABLE];
};

struct Pins
{
	struct PinConfiguration configuration;
	struct SuspendedPinStates suspended;
	struct PinAssignments assignments;
};

struct PeripheralConfiguration
{
	struct I2cConfiguration i2c;
	struct SpiConfiguration spi;
};

struct Configuration
{
	uint8_t unicodeName[32];
	struct UnlockKey unlockKey;
	uint16_t currentLimitMilliamps;
	uint16_t dedicatedChargerAssumedCurrentLimitMilliamps;
	bool isUsbShortedDataLineTestEnabled;
	struct Pins pins;
	struct PeripheralConfiguration peripherals;
};

extern void configurationInitialise(void);
extern uint64_t configurationGetCrc32s(void);
extern uint8_t configurationGetBootIndex(void);
extern void configurationApply(const struct Configuration *configuration);
extern void configurationLoad(struct Configuration *configuration, uint8_t index);
extern bool configurationStore(const struct Configuration *configuration, uint8_t index, bool makeBoot);

#endif
