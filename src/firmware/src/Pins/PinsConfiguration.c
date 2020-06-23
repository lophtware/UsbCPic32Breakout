#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Pins.h"

#include "../Syskey.h"
#include "../Configuration.h"

#define MASK_FOR(bank, includeUnconfigurable) (CONFIGURABLE_##bank##_MASK | ((includeUnconfigurable) ? UNCONFIGURABLE_##bank##_MASK : 0))
#define ANSEL_FOR(bank) ANSEL##bank
#define TRIS_FOR(bank) TRIS##bank
#define LAT_FOR(bank) LAT##bank
#define PORT_FOR(bank) PORT##bank
#define ODC_FOR(bank) ODC##bank
#define CNPU_FOR(bank) CNPU##bank
#define CNPD_FOR(bank) CNPD##bank
#define CNCON_FOR(bank) CNCON##bank
#define CNEN0_FOR(bank) CNEN0##bank
#define CNEN1_FOR(bank) CNEN1##bank
#define CNENISCONTINUOUS_FOR(bank) CNENISCONTINUOUS##bank
#define CNF_FOR(bank) CNF##bank

#define CONCAT(x, y) x##y
#define MASKED_SET(reg, mask, value) \
		{ CONCAT(reg, CLR) = (mask); CONCAT(reg, SET) = ((value) & (mask)); }

#define MASKED_SET_NONSFR(reg, mask, value) \
		(reg) = ((reg) & ~(mask)) | ((value) & (mask));

#define MASKED_GET(reg, mask) ((reg) & (mask))

#define GET_SINGLE_PIN_BANK_STATE(bank, mask, state) \
	{ \
		(state).ansel = MASKED_GET(ANSEL_FOR(bank), (mask)) ? 1 : 0; \
		(state).tris = MASKED_GET(TRIS_FOR(bank), (mask)) ? 1 : 0; \
		(state).lat = MASKED_GET(LAT_FOR(bank), (mask)) ? 1 : 0; \
		(state).port = MASKED_GET(PORT_FOR(bank), (mask)) ? 1 : 0; \
		(state).odc = MASKED_GET(ODC_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnpu = MASKED_GET(CNPU_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnpd = MASKED_GET(CNPD_FOR(bank), (mask)) ? 1 : 0; \
		(state).cncon = MASKED_GET(CNCON_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnen0 = MASKED_GET(CNEN0_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnen1 = MASKED_GET(CNEN1_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnenIsContinuous = MASKED_GET(CNENISCONTINUOUS_FOR(bank), (mask)) ? 1 : 0; \
	}

#define APPLY_SINGLE_PIN_BANK_STATE(bank, pinMask, state) \
	{ \
		uint16_t mask = pinMask & MASK_FOR(bank, false); \
		MASKED_SET(CNEN0_FOR(bank), (mask), 0); \
		MASKED_SET(CNEN1_FOR(bank), (mask), 0); \
		MASKED_SET(TRIS_FOR(bank), (mask), 0xffffffffu); \
		MASKED_SET(ANSEL_FOR(bank), (mask), (state).ansel ? 0xffffffffu : 0); \
		MASKED_SET(CNPD_FOR(bank), (mask), 0); \
		MASKED_SET(CNPU_FOR(bank), (mask), (state).cnpu ? 0xffffffffu : 0); \
		MASKED_SET(CNPD_FOR(bank), (mask), (state).cnpd ? 0xffffffffu : 0); \
		MASKED_SET(LAT_FOR(bank), (mask), (state).lat ? 0xffffffffu : 0); \
		MASKED_SET(ODC_FOR(bank), (mask), (state).odc ? 0xffffffffu : 0); \
		MASKED_SET(TRIS_FOR(bank), (mask), (state).tris ? 0xffffffffu : 0); \
		MASKED_SET_NONSFR(CNENISCONTINUOUS_FOR(bank), (mask), (state).cnenIsContinuous ? 0xffffffffu : 0); \
		MASKED_SET(CNF_FOR(bank), (mask), 0); \
		MASKED_SET(CNEN0_FOR(bank), (mask), (state).cnen0 ? 0xffffffffu : 0); \
		MASKED_SET(CNEN1_FOR(bank), (mask), (state).cnen1 ? 0xffffffffu : 0); \
	}

#define GET_ALL_PIN_BANK_STATE(bank, mask, state) \
	{ \
		(state).ansel = MASKED_GET(ANSEL_FOR(bank), (mask)) ? 1 : 0; \
		(state).tris = MASKED_GET(TRIS_FOR(bank), (mask)) ? 1 : 0; \
		(state).lat = MASKED_GET(LAT_FOR(bank), (mask)) ? 1 : 0; \
		(state).odc = MASKED_GET(ODC_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnpu = MASKED_GET(CNPU_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnpd = MASKED_GET(CNPD_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnen0 = MASKED_GET(CNEN0_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnen1 = MASKED_GET(CNEN1_FOR(bank), (mask)) ? 1 : 0; \
		(state).cnenIsContinuous = MASKED_GET(CNENISCONTINUOUS_FOR(bank), (mask)) ? 1 : 0; \
	}

#define APPLY_ALL_PIN_BANK_STATE(bank, state, includeUnconfigurable) \
	{ \
		uint16_t mask = MASK_FOR(bank, (includeUnconfigurable)); \
		MASKED_SET(CNEN0_FOR(bank), (mask), 0); \
		MASKED_SET(CNEN1_FOR(bank), (mask), 0); \
		MASKED_SET(TRIS_FOR(bank), (mask), 0xffffffffu); \
		MASKED_SET(ANSEL_FOR(bank), (mask), (state).ansel); \
		MASKED_SET(CNPD_FOR(bank), (mask), 0); \
		MASKED_SET(CNPU_FOR(bank), (mask), (state).cnpu); \
		MASKED_SET(CNPD_FOR(bank), (mask), (state).cnpd); \
		MASKED_SET(LAT_FOR(bank), (mask), (state).lat); \
		MASKED_SET(ODC_FOR(bank), (mask), (state).odc); \
		MASKED_SET(CNEN0_FOR(bank), (mask), 0); \
		MASKED_SET(CNEN1_FOR(bank), (mask), 0); \
		MASKED_SET(CNF_FOR(bank), (mask), 0); \
		MASKED_SET(TRIS_FOR(bank), (mask), (state).tris); \
		MASKED_SET_NONSFR(CNENISCONTINUOUS_FOR(bank), (mask), (state).cnenIsContinuous); \
		MASKED_SET(CNEN0_FOR(bank), (mask), (state).cnen0); \
		MASKED_SET(CNEN1_FOR(bank), (mask), (state).cnen1); \
	}

extern void __attribute__((interrupt(), vector(_CHANGE_NOTICE_A_VECTOR), nomips16)) _pinsChangedInterruptA(void);
extern void __attribute__((interrupt(), vector(_CHANGE_NOTICE_B_VECTOR), nomips16)) _pinsChangedInterruptB(void);
extern void __attribute__((interrupt(), vector(_CHANGE_NOTICE_C_VECTOR), nomips16)) _pinsChangedInterruptC(void);

static void pinsApplyAllConfiguration(bool isBooting);
static void calculateMasksForAssignedPins(void);
static void applyPinBankState(bool isBooting);
static void applyPinPeripheralInputMap(void);
static void ppsUnlock(void);
static void ppsLock(void);
static void applyPinPeripheralOutputMap(void);
static void getPinBankState(struct PinBankState banks[]);
static void getPinPeripheralInputMap(struct PinPeripheralInputMap *map);
static void getPinPeripheralOutputMap(struct PinPeripheralOutputMap *map);
static uint8_t pinToRpIndex(const struct Pin pin);
static union RpinrMap getPinPeripheralInput(uint32_t rpIndex);
static uint8_t getPinPeripheralOutput(uint8_t rpIndex);
static void applyPinPeripheralInput(uint8_t rpIndex, union RpinrMap rpinr);
static void applyPinPeripheralOutput(uint8_t rpIndex, uint8_t rpor);

const struct Pin pinsConfigurable[PINS_NUMBER_CONFIGURABLE] =
{
	{ .bank = 0, .index = 2 },
	{ .bank = 0, .index = 3 },
	{ .bank = 0, .index = 4 },
	{ .bank = 1, .index = 0 },
	{ .bank = 1, .index = 1 },
	{ .bank = 1, .index = 2 },
	{ .bank = 1, .index = 3 },
	{ .bank = 1, .index = 4 },
	{ .bank = 1, .index = 5 },
	{ .bank = 1, .index = 7 },
	{ .bank = 2, .index = 9 }
};

static uint32_t CNENISCONTINUOUSA;
static uint32_t CNENISCONTINUOUSB;
static uint32_t CNENISCONTINUOUSC;

const struct PinConfiguration *pins;
uint32_t pinsAssignedMaskA;
uint32_t pinsAssignedMaskB;
uint32_t pinsAssignedMaskC;

TaskHandle_t pinsChangedTaskHandle;
uint32_t pinsChangedTimestamp;

void pinsInitialise(void)
{
	const int priority = 2;
	IPC2bits.CNAIP = priority;
	IPC2bits.CNAIS = 0;
	IPC2bits.CNBIP = priority;
	IPC2bits.CNBIS = 0;
	IPC2bits.CNCIP = priority;
	IPC2bits.CNCIS = 0;

	IEC0SET = _IEC0_CNAIE_MASK | _IEC0_CNBIE_MASK | _IEC0_CNCIE_MASK;
}

void pinsApplyBootConfiguration(const struct PinConfiguration *newPins)
{
	CNCONA = _CNCONA_ON_MASK | _CNCONA_CNSTYLE_MASK;
	CNCONB = _CNCONB_ON_MASK | _CNCONB_CNSTYLE_MASK;
	CNCONC = _CNCONC_ON_MASK | _CNCONC_CNSTYLE_MASK;

	if (!newPins)
		return;

	pins = newPins;
	pinsApplyAllConfiguration(true);
}

static void pinsApplyAllConfiguration(bool isBooting)
{
	calculateMasksForAssignedPins();
	if (!isBooting)
		taskENTER_CRITICAL();

	applyPinBankState(isBooting);
	applyPinPeripheralInputMap();
	applyPinPeripheralOutputMap();

	if (!isBooting)
		taskEXIT_CRITICAL();
}

static void calculateMasksForAssignedPins(void)
{
	pinsAssignedMaskA = 0;
	pinsAssignedMaskB = 0;
	pinsAssignedMaskC = 0;

	for (int i = 0; i < sizeof(pins->pinMaskMap) / sizeof(struct PinMaskMap); i++)
	{
		switch (pins->pinMaskMap[i].bank)
		{
			case 0:
				pinsAssignedMaskA |= pins->pinMaskMap[i].mask;
				break;

			case 1:
				pinsAssignedMaskB |= pins->pinMaskMap[i].mask;
				break;

			case 2:
				pinsAssignedMaskC |= pins->pinMaskMap[i].mask;
				break;

			default:
				break;
		}
	}

	pinsAssignedMaskA &= CONFIGURABLE_A_MASK;
	pinsAssignedMaskB &= CONFIGURABLE_B_MASK;
	pinsAssignedMaskC &= CONFIGURABLE_C_MASK;
}

static void applyPinBankState(bool isBooting)
{
	APPLY_ALL_PIN_BANK_STATE(A, pins->bankStates[0], isBooting);
	APPLY_ALL_PIN_BANK_STATE(B, pins->bankStates[1], isBooting);
	APPLY_ALL_PIN_BANK_STATE(C, pins->bankStates[2], isBooting);
}

static void applyPinPeripheralInputMap(void)
{
	syskeyUnlockThen(ppsUnlock);
	RPINR1 = pins->peripheralInputMap.rpinr1;
	RPINR2 = pins->peripheralInputMap.rpinr2;
	RPINR3 = pins->peripheralInputMap.rpinr3;
	RPINR5 = pins->peripheralInputMap.rpinr5;
	RPINR6 = pins->peripheralInputMap.rpinr6;
	RPINR7 = pins->peripheralInputMap.rpinr7;
	RPINR8 = pins->peripheralInputMap.rpinr8;
	RPINR9 = pins->peripheralInputMap.rpinr9;
	RPINR10 = pins->peripheralInputMap.rpinr10;
	RPINR11 = pins->peripheralInputMap.rpinr11;
	RPINR12 = pins->peripheralInputMap.rpinr12;
	syskeyUnlockThen(ppsLock);
}

static void ppsUnlock(void)
{
	RPCONbits.IOLOCK = 0;
}

static void ppsLock(void)
{
	RPCONbits.IOLOCK = 1;
}

static void applyPinPeripheralOutputMap(void)
{
	syskeyUnlockThen(ppsUnlock);
	MASKED_SET(RPOR0, CONFIGURABLE_RPOR0_MASK, pins->peripheralOutputMap.rpor0);
	MASKED_SET(RPOR1, CONFIGURABLE_RPOR1_MASK, pins->peripheralOutputMap.rpor1);
	MASKED_SET(RPOR2, CONFIGURABLE_RPOR2_MASK, pins->peripheralOutputMap.rpor2);
	MASKED_SET(RPOR4, CONFIGURABLE_RPOR4_MASK, pins->peripheralOutputMap.rpor4);
	syskeyUnlockThen(ppsLock);
}

void pinsApplyConfiguration(const struct PinConfiguration *newPins)
{
	if (!newPins)
		return;

	pins = newPins;
	pinsApplyAllConfiguration(false);
}

void pinsGetConfiguration(struct PinConfiguration *pinsOut)
{
	if (!pinsOut)
		return;

	memcpy(pinsOut->pinMaskMap, &pins->pinMaskMap, sizeof(pins->pinMaskMap));
	getPinBankState(pinsOut->bankStates);
	getPinPeripheralInputMap(&pinsOut->peripheralInputMap);
	getPinPeripheralOutputMap(&pinsOut->peripheralOutputMap);
}

static void getPinBankState(struct PinBankState banks[])
{
	GET_ALL_PIN_BANK_STATE(A, 0xffffffffu, banks[0]);
	GET_ALL_PIN_BANK_STATE(B, 0xffffffffu, banks[1]);
	GET_ALL_PIN_BANK_STATE(C, 0xffffffffu, banks[2]);
}

static void getPinPeripheralInputMap(struct PinPeripheralInputMap *map)
{
	map->rpinr1 = RPINR1;
	map->rpinr2 = RPINR2;
	map->rpinr3 = RPINR3;
	map->rpinr5 = RPINR5;
	map->rpinr6 = RPINR6;
	map->rpinr7 = RPINR7;
	map->rpinr8 = RPINR8;
	map->rpinr9 = RPINR9;
	map->rpinr10 = RPINR10;
	map->rpinr11 = RPINR11;
	map->rpinr12 = RPINR12;
}

static void getPinPeripheralOutputMap(struct PinPeripheralOutputMap *map)
{
	map->rpor0 = RPOR0;
	map->rpor1 = RPOR1;
	map->rpor2 = RPOR2;
	map->rpor4 = RPOR4;
}

void pinsGetState(const struct Pin pin, struct PinState *state)
{
	if (!state)
		return;

	if (pin.bank == 0)
	{
		GET_SINGLE_PIN_BANK_STATE(A, 1 << pin.index, state->bits);
		uint8_t rpIndex = pinToRpIndex(pin);
		state->rpinr = getPinPeripheralInput(rpIndex);
		state->rpor = getPinPeripheralOutput(rpIndex);
	}
	else if (pin.bank == 1)
	{
		GET_SINGLE_PIN_BANK_STATE(B, 1 << pin.index, state->bits);
		uint8_t rpIndex = pinToRpIndex(pin);
		state->rpinr = getPinPeripheralInput(rpIndex);
		state->rpor = getPinPeripheralOutput(rpIndex);
	}
	else if (pin.bank == 2)
	{
		GET_SINGLE_PIN_BANK_STATE(C, 1 << pin.index, state->bits);
		uint8_t rpIndex = pinToRpIndex(pin);
		state->rpinr = getPinPeripheralInput(rpIndex);
		state->rpor = getPinPeripheralOutput(rpIndex);
	}
	else
	{
		GET_SINGLE_PIN_BANK_STATE(A, 0, state->bits);
		state->rpor = 0;
		state->rpinr.raw = 0;
	}
}

static uint8_t pinToRpIndex(const struct Pin pin)
{
	uint8_t pinIndex = (pin.bank << 4) | (pin.index & 0x0f);
	switch (pinIndex)
	{
		case 0x00: return 1;
		case 0x01: return 2;
		case 0x02: return 3;
		case 0x03: return 4;
		case 0x04: return 5;

		case 0x10: return 6;
		case 0x11: return 7;
		case 0x12: return 8;
		case 0x13: return 9;
		case 0x14: return 10;
		case 0x15: return 11;
		case 0x17: return 12;
		case 0x18: return 13;
		case 0x19: return 14;
		case 0x1d: return 15;
		case 0x1e: return 16;
		case 0x1f: return 17;

		case 0x29: return 18;
	}

	return 0xff;
}

static union RpinrMap getPinPeripheralInput(uint32_t rpIndex)
{
	union RpinrMap map =
	{
		.peripherals =
		{
			.int4 = (RPINR1bits.INT4R == rpIndex) ? 1 : 0,
			.icm1 = (RPINR2bits.ICM1R == rpIndex) ? 1 : 0,
			.icm2 = (RPINR2bits.ICM2R == rpIndex) ? 1 : 0,
			.icm3 = (RPINR3bits.ICM3R == rpIndex) ? 1 : 0,
			.icm4 = (RPINR3bits.ICM4R == rpIndex) ? 1 : 0,
			.ocfa = (RPINR5bits.OCFAR == rpIndex) ? 1 : 0,
			.ocfb = (RPINR5bits.OCFBR == rpIndex) ? 1 : 0,
			.tckia = (RPINR6bits.TCKIAR == rpIndex) ? 1 : 0,
			.tckib = (RPINR6bits.TCKIBR == rpIndex) ? 1 : 0,
			.icm5 = (RPINR7bits.ICM5R == rpIndex) ? 1 : 0,
			.icm6 = (RPINR7bits.ICM6R == rpIndex) ? 1 : 0,
			.icm7 = (RPINR7bits.ICM7R == rpIndex) ? 1 : 0,
			.icm8 = (RPINR7bits.ICM8R == rpIndex) ? 1 : 0,
			.icm9 = (RPINR8bits.ICM9R == rpIndex) ? 1 : 0,
			.u3rx = (RPINR8bits.U3RXR == rpIndex) ? 1 : 0,
			.u2rx = (RPINR9bits.U2RXR == rpIndex) ? 1 : 0,
			._u2cts = (RPINR9bits.U2CTSR == rpIndex) ? 1 : 0,
			._u3cts = (RPINR10bits.U3CTSR == rpIndex) ? 1 : 0,
			.sdi2 = (RPINR11bits.SDI2R == rpIndex) ? 1 : 0,
			.sck2in = (RPINR11bits.SCK2INR == rpIndex) ? 1 : 0,
			.ss2in = (RPINR11bits.SS2INR == rpIndex) ? 1 : 0,
			.clcina = (RPINR12bits.CLCINAR == rpIndex) ? 1 : 0,
			.clcinb = (RPINR12bits.CLCINBR == rpIndex) ? 1 : 0
		}
	};

	return map;
}

static uint8_t getPinPeripheralOutput(uint8_t rpIndex)
{
	uint32_t shift = 8 * (--rpIndex & 0x03);
	switch (rpIndex / 4)
	{
		case 0: return (RPOR0 >> shift) & 0x1f;
		case 1: return (RPOR1 >> shift) & 0x1f;
		case 2: return (RPOR2 >> shift) & 0x1f;
		case 3: return (RPOR3 >> shift) & 0x1f;
		case 4: return (RPOR4 >> shift) & 0x1f;
		case 5: return (RPOR5 >> shift) & 0x1f;
	}

	return 0;
}

void pinsSetState(const struct Pin pin, const struct PinState *state)
{
	if (!state)
		return;

	if (pin.bank == 0)
	{
		APPLY_SINGLE_PIN_BANK_STATE(A, 1 << pin.index, state->bits);
		if ((1 << pin.index) & CONFIGURABLE_A_MASK)
		{
			uint8_t rpIndex = pinToRpIndex(pin);
			applyPinPeripheralInput(rpIndex, state->rpinr);
			applyPinPeripheralOutput(rpIndex, state->rpor);
		}
	}
	else if (pin.bank == 1)
	{
		APPLY_SINGLE_PIN_BANK_STATE(B, 1 << pin.index, state->bits);
		if ((1 << pin.index) & CONFIGURABLE_B_MASK)
		{
			uint8_t rpIndex = pinToRpIndex(pin);
			applyPinPeripheralInput(rpIndex, state->rpinr);
			applyPinPeripheralOutput(rpIndex, state->rpor);
		}
	}
	else if (pin.bank == 2)
	{
		APPLY_SINGLE_PIN_BANK_STATE(C, 1 << pin.index, state->bits);
		if ((1 << pin.index) & CONFIGURABLE_C_MASK)
		{
			uint8_t rpIndex = pinToRpIndex(pin);
			applyPinPeripheralInput(rpIndex, state->rpinr);
			applyPinPeripheralOutput(rpIndex, state->rpor);
		}
	}
}

static void applyPinPeripheralInput(uint8_t rpIndex, union RpinrMap rpinr)
{
	syskeyUnlockThen(ppsUnlock);

	if (rpIndex > 18)
		rpIndex = 0;

	if (rpinr.peripherals.int4)
		RPINR1bits.INT4R = rpIndex;
	else if (RPINR1bits.INT4R == rpIndex)
		RPINR1bits.INT4R = 0;

	if (rpinr.peripherals.icm1)
		RPINR2bits.ICM1R = rpIndex;
	else if (RPINR2bits.ICM1R == rpIndex)
		RPINR2bits.ICM1R = 0;

	if (rpinr.peripherals.icm2)
		RPINR2bits.ICM2R = rpIndex;
	else if (RPINR2bits.ICM2R == rpIndex)
		RPINR2bits.ICM2R = 0;

	if (rpinr.peripherals.icm3)
		RPINR3bits.ICM3R = rpIndex;
	else if (RPINR3bits.ICM3R == rpIndex)
		RPINR3bits.ICM3R = 0;

	if (rpinr.peripherals.icm4)
		RPINR3bits.ICM4R = rpIndex;
	else if (RPINR3bits.ICM4R == rpIndex)
		RPINR3bits.ICM4R = 0;

	if (rpinr.peripherals.ocfa)
		RPINR5bits.OCFAR = rpIndex;
	else if (RPINR5bits.OCFAR == rpIndex)
		RPINR5bits.OCFAR = 0;

	if (rpinr.peripherals.ocfb)
		RPINR5bits.OCFBR = rpIndex;
	else if (RPINR5bits.OCFBR == rpIndex)
		RPINR5bits.OCFBR = 0;

	if (rpinr.peripherals.tckia)
		RPINR6bits.TCKIAR = rpIndex;
	else if (RPINR6bits.TCKIAR == rpIndex)
		RPINR6bits.TCKIAR = 0;

	if (rpinr.peripherals.tckib)
		RPINR6bits.TCKIBR = rpIndex;
	else if (RPINR6bits.TCKIBR == rpIndex)
		RPINR6bits.TCKIBR = 0;

	if (rpinr.peripherals.icm5)
		RPINR7bits.ICM5R = rpIndex;
	else if (RPINR7bits.ICM5R == rpIndex)
		RPINR7bits.ICM5R = 0;

	if (rpinr.peripherals.icm6)
		RPINR7bits.ICM6R = rpIndex;
	else if (RPINR7bits.ICM6R == rpIndex)
		RPINR7bits.ICM6R = 0;

	if (rpinr.peripherals.icm7)
		RPINR7bits.ICM7R = rpIndex;
	else if (RPINR7bits.ICM7R == rpIndex)
		RPINR7bits.ICM7R = 0;

	if (rpinr.peripherals.icm8)
		RPINR7bits.ICM8R = rpIndex;
	else if (RPINR7bits.ICM8R == rpIndex)
		RPINR7bits.ICM8R = 0;

	if (rpinr.peripherals.icm9)
		RPINR8bits.ICM9R = rpIndex;
	else if (RPINR8bits.ICM9R == rpIndex)
		RPINR8bits.ICM9R = 0;

	if (rpinr.peripherals.u3rx)
		RPINR8bits.U3RXR = rpIndex;
	else if (RPINR8bits.U3RXR == rpIndex)
		RPINR8bits.U3RXR = 0;

	if (rpinr.peripherals.u2rx)
		RPINR9bits.U2RXR = rpIndex;
	else if (RPINR9bits.U2RXR == rpIndex)
		RPINR9bits.U2RXR = 0;

	if (rpinr.peripherals._u2cts)
		RPINR9bits.U2CTSR = rpIndex;
	else if (RPINR9bits.U2CTSR == rpIndex)
		RPINR9bits.U2CTSR = 0;

	if (rpinr.peripherals._u3cts)
		RPINR10bits.U3CTSR = rpIndex;
	else if (RPINR10bits.U3CTSR == rpIndex)
		RPINR10bits.U3CTSR = 0;

	if (rpinr.peripherals.sck2in)
		RPINR11bits.SCK2INR = rpIndex;
	else if (RPINR11bits.SCK2INR == rpIndex)
		RPINR11bits.SCK2INR = 0;

	if (rpinr.peripherals.sdi2)
		RPINR11bits.SDI2R = rpIndex;
	else if (RPINR11bits.SDI2R == rpIndex)
		RPINR11bits.SDI2R = 0;

	if (rpinr.peripherals.ss2in)
		RPINR11bits.SS2INR = rpIndex;
	else if (RPINR11bits.SS2INR == rpIndex)
		RPINR11bits.SS2INR = 0;

	if (rpinr.peripherals.clcina)
		RPINR12bits.CLCINAR = rpIndex;
	else if (RPINR12bits.CLCINAR == rpIndex)
		RPINR12bits.CLCINAR = 0;

	if (rpinr.peripherals.clcinb)
		RPINR12bits.CLCINBR = rpIndex;
	else if (RPINR12bits.CLCINBR == rpIndex)
		RPINR12bits.CLCINBR = 0;

	syskeyUnlockThen(ppsLock);
}

static void applyPinPeripheralOutput(uint8_t rpIndex, uint8_t rpor)
{
	syskeyUnlockThen(ppsUnlock);

	uint32_t shift = 8 * (--rpIndex & 0x03);
	uint32_t mask = 0x1f << shift;
	switch (rpIndex / 4)
	{
		case 0:
			RPOR0 = (RPOR0 & ~mask) | (rpor << shift);
			break;

		case 1:
			RPOR1 = (RPOR1 & ~mask) | (rpor << shift);
			break;

		case 2:
			RPOR2 = (RPOR2 & ~mask) | (rpor << shift);
			break;

		case 3:
			RPOR3 = (RPOR3 & ~mask) | (rpor << shift);
			break;

		case 4:
			RPOR4 = (RPOR4 & ~mask) | (rpor << shift);
			break;

		case 5:
			RPOR5 = (RPOR5 & ~mask) | (rpor << shift);
			break;
	}

	syskeyUnlockThen(ppsLock);
}

void pinsModifyBankStateForPinState(struct PinBankState *bankState, const struct Pin pin, const struct PinState *pinState)
{
	if (!bankState || !pinState)
		return;

	uint32_t pinMask = pinsGetMaskFrom(pin);
	if (!pinMask)
		return;

	MASKED_SET_NONSFR(bankState->ansel, pinMask, pinState->bits.ansel ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->cnen0, pinMask, pinState->bits.cnen0 ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->cnen1, pinMask, pinState->bits.cnen1 ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->cnenIsContinuous, pinMask, pinState->bits.cnenIsContinuous ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->cnpd, pinMask, pinState->bits.cnpd ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->cnpu, pinMask, pinState->bits.cnpu ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->lat, pinMask, pinState->bits.lat ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->odc, pinMask, pinState->bits.odc ? pinMask : 0);
	MASKED_SET_NONSFR(bankState->tris, pinMask, pinState->bits.tris ? pinMask : 0);
}

uint32_t pinsGetMaskFrom(const struct Pin pin)
{
	if (pin.bank > 2)
		return 0;

	static const uint32_t configurableMasks[] = {CONFIGURABLE_A_MASK, CONFIGURABLE_B_MASK, CONFIGURABLE_C_MASK};
	return (1 << pin.index) & configurableMasks[pin.bank];
}
