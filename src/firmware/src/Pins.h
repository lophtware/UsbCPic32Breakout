#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_PINS_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_PINS_H
#include <stdint.h>
#include <stdbool.h>
#include "FreeRtos.h"
#include "BoardVariants/Pins.h"

#define PIN_NONE { .bank = 0xff, .index = 0xff }

#define PIN_MASK(b) (1 << (b))
#define DEFAULT_MASK(reg) (UNCONFIGURABLE_##reg##_MASK | CONFIGURABLE_##reg##_MASK)

#define RA(x) PIN_MASK(x)
#define RB(x) PIN_MASK(x)
#define RC(x) PIN_MASK(x)

#define PIN_SINGLE_OP(reg, op, pin) \
	{ \
		switch ((pin).bank) \
		{ \
			case 0: \
				reg##A##op = (1 << (pin).index) & CONFIGURABLE_A_MASK; \
				break; \
	\
			case 1: \
				reg##B##op = (1 << (pin).index) & CONFIGURABLE_B_MASK; \
				break; \
 	\
			case 2: \
				reg##C##op = (1 << (pin).index) & CONFIGURABLE_C_MASK; \
				break; \
 	\
			default: \
				break; \
		} \
	}

struct PinMaskMap
{
	uint8_t bank;
	uint16_t mask;
};

struct PinBankState
{
	uint16_t ansel;
	uint16_t tris;
	uint16_t lat;
	uint16_t odc;
	uint16_t cnpu;
	uint16_t cnpd;
	uint16_t cnen0;
	uint16_t cnen1;
	uint16_t cnenIsContinuous;
};

struct PinPeripheralInputMap
{
	uint32_t rpinr1;
	uint32_t rpinr2;
	uint32_t rpinr3;
	uint32_t rpinr5;
	uint32_t rpinr6;
	uint32_t rpinr7;
	uint32_t rpinr8;
	uint32_t rpinr9;
	uint32_t rpinr10;
	uint32_t rpinr11;
	uint32_t rpinr12;
};

struct PinPeripheralOutputMap
{
	uint32_t rpor0;
	uint32_t rpor1;
	uint32_t rpor2;
	uint32_t rpor3;
	uint32_t rpor4;
};

struct PinConfiguration
{
	struct PinMaskMap pinMaskMap[PINS_NUMBER_CONFIGURABLE];
	struct PinBankState bankStates[3];
	struct PinPeripheralInputMap peripheralInputMap;
	struct PinPeripheralOutputMap peripheralOutputMap;
};

struct Pin
{
	uint8_t bank;
	uint8_t index;
};

union RpinrMap
{
	struct
	{
		unsigned int int4 : 1;
		unsigned int icm1 : 1;
		unsigned int icm2 : 1;
		unsigned int icm3 : 1;
		unsigned int icm4 : 1;
		unsigned int ocfa : 1;
		unsigned int ocfb : 1;
		unsigned int tckia : 1;
		unsigned int tckib : 1;
		unsigned int icm5 : 1;
		unsigned int icm6 : 1;
		unsigned int icm7 : 1;
		unsigned int icm8 : 1;
		unsigned int icm9 : 1;
		unsigned int u3rx : 1;
		unsigned int u2rx : 1;
		unsigned int _u2cts : 1;
		unsigned int _u3cts : 1;
		unsigned int sdi2 : 1;
		unsigned int sck2in : 1;
		unsigned int ss2in : 1;
		unsigned int clcina : 1;
		unsigned int clcinb : 1;
	} peripherals;
	
	uint32_t raw;
};

struct PinState
{
	struct
	{
		unsigned int ansel : 1;
		unsigned int tris : 1;
		unsigned int lat : 1;
		unsigned int port : 1;
		unsigned int odc : 1;
		unsigned int cnpu : 1;
		unsigned int cnpd : 1;
		unsigned int cncon : 1;
		unsigned int cnen0 : 1;
		unsigned int cnen1 : 1;
		unsigned int cnenIsContinuous : 1;
	} bits;

	union RpinrMap rpinr;
	uint8_t rpor;
};

extern const struct Pin pinsConfigurable[PINS_NUMBER_CONFIGURABLE];

extern void pinsInitialise(void);
extern void pinsApplyBootConfiguration(const struct PinConfiguration *pins);
extern void pinsApplyConfiguration(const struct PinConfiguration *pins);
extern void pinsGetConfiguration(struct PinConfiguration *pins);
extern void pinsGetState(const struct Pin pin, struct PinState *state);
extern void pinsSetState(const struct Pin pin, const struct PinState *state);
extern void pinsModifyBankStateForPinState(struct PinBankState *bankState, const struct Pin pin, const struct PinState *pinState);
extern uint32_t pinsGetMaskFrom(const struct Pin pin);
extern void pinsMaskedLatSet(uint16_t mask);
extern void pinsMaskedLatClear(uint16_t mask);
extern void pinsMaskedLatToggle(uint16_t mask);
extern void pinsMaskedLatLoad(uint16_t mask);
extern void pinsOnChangedNotify(TaskHandle_t pinsChanged);
extern uint32_t pinsGetLastOnChangedTimestamp(void);
extern void pinsChangedReset(void);
extern void pinsChangedResetNonContinuous(uint16_t a, uint16_t b, uint16_t c);

static inline void pinsLatSet(const struct Pin pin)
{
	PIN_SINGLE_OP(LAT, SET, pin);
}

static inline void pinsLatClear(const struct Pin pin)
{
	PIN_SINGLE_OP(LAT, CLR, pin);
}

static inline bool pinsEqual(const struct Pin a, const struct Pin b)
{
	return a.bank == b.bank && a.index == b.index;
}

static const struct Pin noPin = PIN_NONE;

#endif
