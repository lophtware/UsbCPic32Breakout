#include <xc.h>
#include <stdint.h>

#include "../../Adc.h"

#include "../UsbCc.h"

#include "UsbCcAdc.h"

#define CCSTATE_DEBUG_FLAG 0x40
#define CCSTATE_CC2_FLAG 0x20
#define CCSTATE_CC1_FLAG 0x10

enum RpState
{
	RpState_Unattached,
	RpState_Default,
	RpState_1A5,
	RpState_3A
};

enum CcState
{
	CcState_Unattached,

	CcState_Cc1_Default = CCSTATE_CC1_FLAG | 0x00,
	CcState_Cc1_1A5 = CCSTATE_CC1_FLAG | 0x01,
	CcState_Cc1_3A = CCSTATE_CC1_FLAG | 0x02,
	CcState_Cc2_Default = CCSTATE_CC2_FLAG | 0x00,
	CcState_Cc2_1A5 = CCSTATE_CC2_FLAG | 0x01,
	CcState_Cc2_3A = CCSTATE_CC2_FLAG | 0x02,

	CcState_Cc1_Default_Debug = CCSTATE_DEBUG_FLAG | CCSTATE_CC1_FLAG | 0x00,
	CcState_Cc1_1A5_Debug = CCSTATE_DEBUG_FLAG | CCSTATE_CC1_FLAG | 0x01,
	CcState_Cc1_3A_Debug = CCSTATE_DEBUG_FLAG | CCSTATE_CC1_FLAG | 0x02,
	CcState_Cc2_Default_Debug = CCSTATE_DEBUG_FLAG | CCSTATE_CC2_FLAG | 0x00,
	CcState_Cc2_1A5_Debug = CCSTATE_DEBUG_FLAG | CCSTATE_CC2_FLAG | 0x01,
	CcState_Cc2_3A_Debug = CCSTATE_DEBUG_FLAG | CCSTATE_CC2_FLAG | 0x02,

	CcState_Invalid = 0x80
};

static inline enum CcState decodeCcVoltages(uint16_t cc1, uint16_t cc2);
static inline enum RpState decodeCcVoltage(uint16_t cc);

void usbCcAdcDecodeCcVoltagesIntoFlags(struct UsbCcEvent *event, uint16_t cc1, uint16_t cc2)
{
	usbCcAdcCurrentLimitMilliamps = 100;
	event->as.flagsChanged.flags.all = 0;
	event->as.flagsChanged.flags.bits.isAttached = 1;
	event->as.flagsChanged.flags.bits.isCcVoltageOk = 1;
	event->as.flagsChanged.flags.bits.isOrientationOk = 1;

	enum CcState ccState = decodeCcVoltages(cc1, cc2);
	switch (ccState)
	{
		case CcState_Cc1_Default:
		case CcState_Cc1_Default_Debug:
		case CcState_Cc2_Default:
		case CcState_Cc2_Default_Debug:
			usbCcAdcCurrentLimitMilliamps = 500;
			event->as.flagsChanged.flags.bits.currentLimit = 1;
			break;

		case CcState_Cc1_1A5:
		case CcState_Cc1_1A5_Debug:
		case CcState_Cc2_1A5:
		case CcState_Cc2_1A5_Debug:
			usbCcAdcCurrentLimitMilliamps = 1500;
			event->as.flagsChanged.flags.bits.currentLimit = 2;
			break;

		case CcState_Cc1_3A:
		case CcState_Cc1_3A_Debug:
		case CcState_Cc2_3A:
		case CcState_Cc2_3A_Debug:
			usbCcAdcCurrentLimitMilliamps = 3000;
			event->as.flagsChanged.flags.bits.currentLimit = 3;
			break;

		case CcState_Unattached:
			event->as.flagsChanged.flags.bits.isAttached = 0;
			break;

		case CcState_Invalid:
		default:
			event->as.flagsChanged.flags.bits.isCcVoltageOk = 0;
			event->as.flagsChanged.flags.bits.isOrientationOk = 0;
			break;
	}

	if (ccState & CCSTATE_CC1_FLAG)
		event->as.flagsChanged.flags.bits.isOrientationCc1A5 = 1;

	if (ccState & CCSTATE_CC2_FLAG)
		event->as.flagsChanged.flags.bits.isOrientationCc2B5 = 1;

	event->as.flagsChanged.flags.bits.isDebugAccessory = (ccState & CCSTATE_DEBUG_FLAG) ? 1 : 0;
	event->as.flagsChanged.flags.bits.isVbusVoltageOk = U1OTGSTATbits.VBUSVD ? 1 : 0;
}

static inline enum CcState decodeCcVoltages(uint16_t cc1, uint16_t cc2)
{
	enum RpState rp1 = decodeCcVoltage(cc1);
	enum RpState rp2 = decodeCcVoltage(cc2);

	if (rp1 == RpState_Unattached)
	{
		if (rp2 == RpState_Unattached)
			return CcState_Unattached;

		if (rp2 == RpState_Default)
			return CcState_Cc2_Default;

		if (rp2 == RpState_1A5)
			return CcState_Cc2_1A5;

		return CcState_Cc2_3A;
	}

	if (rp2 == RpState_Unattached)
	{
		if (rp1 == RpState_Default)
			return CcState_Cc1_Default;

		if (rp1 == RpState_1A5)
			return CcState_Cc1_1A5;

		return CcState_Cc1_3A;
	}

	if (rp1 == RpState_3A)
	{
		if (rp2 == RpState_1A5)
			return CcState_Cc1_Default_Debug;

		if (rp2 == RpState_Default)
			return CcState_Cc1_3A_Debug;
	}

	if (rp2 == RpState_3A)
	{
		if (rp1 == RpState_1A5)
			return CcState_Cc2_Default_Debug;

		if (rp1 == RpState_Default)
			return CcState_Cc2_3A_Debug;
	}

	if (rp1 == RpState_1A5 && rp2 == RpState_Default)
		return CcState_Cc1_1A5_Debug;

	if (rp1 == RpState_Default && rp2 == RpState_1A5)
		return CcState_Cc2_1A5_Debug;

	return CcState_Invalid;
}

static inline enum RpState decodeCcVoltage(uint16_t cc)
{
	if (cc >= ADC_VOLTAGE_10BIT(1.23))
		return RpState_3A;

	if (cc >= ADC_VOLTAGE_10BIT(0.66))
		return RpState_1A5;

	if (cc >= ADC_VOLTAGE_10BIT(0.2))
		return RpState_Default;

	return RpState_Unattached;
}
