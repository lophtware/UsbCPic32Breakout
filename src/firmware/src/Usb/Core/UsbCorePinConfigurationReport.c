#include <xc.h>
#include <stdint.h>

#include "../../UnlockKey.h"
#include "../../Pins.h"
#include "../../FreeRtos.h"

#include "UsbCoreInterface.h"

#define MASKED_SET(dest, src, mask) \
	{ \
		(dest) &= ~(mask); \
		(dest) |= (src) & (mask); \
	}

#define BYTES_TO_UINT32(ll, lh, hl, hh) (((uint32_t) (ll)) << 0) | (((uint32_t) (lh)) << 8) | (((uint32_t) (hl)) << 16) | (((uint32_t) (hh)) << 24)

#define SUSPEND_BEHAVIOUR_UNCHANGED 0
#define SUSPEND_BEHAVIOUR_INPUT 1
#define SUSPEND_BEHAVIOUR_OUTPUT_0 2
#define SUSPEND_BEHAVIOUR_OUTPUT_1 3

static struct PinAssignment *usbCoreGetExistingAssignment(uint8_t pinPacked);
static void constructUnsuspendedPinState(struct PinState *pinState, uint32_t args);
static void setPinMaskMapInCurrentConfiguration(uint8_t pinBank, uint32_t pinMask, uint32_t args);
static void unsetPinMaskMapInCurrentConfiguration(uint8_t pinBank, uint32_t pinMask);
static void setSuspendedPinBankState(struct PinBankState *suspended, const struct PinBankState *unsuspended, uint32_t pinMask, uint8_t suspendBehaviour);

void usbCoreOnPinConfigurationReportReceived(const uint8_t *report)
{
	uint8_t pinPacked = report[0] & ~CORE_PIN_CONFIGURATION_REPORT_ID_MASK;
	uint8_t pinBank = (pinPacked >> 4) & 0x0f;
	uint8_t pinIndex = (pinPacked >> 0) & 0x0f;
	struct UnlockKey suppliedKey =
	{
		.as =
		{
			.bytes =
			{
				report[1],
				report[2],
				report[3],
				report[4],
				report[5],
				report[6],
				report[7],
				report[8]
			}
		}
	};
	uint8_t suspendBehaviour = report[9] & 0xe7;
	uint8_t interface = report[10];
	uint8_t function = report[11];
	uint64_t functionArgs =
		BYTES_TO_UINT32(function, report[12], report[13], report[14]) |
		(((uint64_t) BYTES_TO_UINT32(report[15], report[16], report[17], report[18])) << 32);

	if (!unlockKeyMatches(&suppliedKey))
	{
		usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_NACK_UNLOCK_KEY);
		return;
	}

	if (interface >= NUMBER_OF_INTERFACES)
	{
		usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_NACK_UNKNOWN_INTERFACE(interface));
		return;
	}

	struct Pin pin = { .bank = pinBank, .index = pinIndex };
	uint32_t pinMask = pinsGetMaskFrom(pin);
	struct PinAssignment *pinAssignment = usbCoreGetExistingAssignment(pinPacked);
	if (!pinMask || !pinAssignment)
	{
		usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_NACK_UNKNOWN_PIN(pinPacked));
		return;
	}

	taskENTER_CRITICAL();

	struct PinState pinState;
	bool successfullyAssigned = usbInterfaces[interface]->assignPin
		? usbInterfaces[interface]->assignPin(&pinState, pin, functionArgs)
		: false;

	if (!successfullyAssigned)
	{
		taskEXIT_CRITICAL();
		usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_NACK_UNKNOWN_PIN(pinPacked));
		return;
	}

	if (pinAssignment->interface != interface && pinAssignment->interface < NUMBER_OF_INTERFACES)
	{
		if (usbInterfaces[pinAssignment->interface]->unassignPin)
			usbInterfaces[pinAssignment->interface]->unassignPin(pin);
	}

	pinAssignment->suspendBehaviour = suspendBehaviour;
	pinAssignment->interface = interface;
	pinAssignment->args = functionArgs;

	if (interface == CORE_INTERFACE_ID)
		setPinMaskMapInCurrentConfiguration(pinBank, pinMask, (uint32_t) functionArgs);
	else
		unsetPinMaskMapInCurrentConfiguration(pinBank, pinMask);

	struct PinBankState *unsuspended = &usbCurrentConfiguration.pins.configuration.bankStates[pinBank];
	pinsModifyBankStateForPinState(unsuspended, pin, &pinState);
	pinsSetState(pin, &pinState);

	struct PinBankState *suspended = &usbCurrentConfiguration.pins.suspended.bankStates[pinBank];
	setSuspendedPinBankState(suspended, unsuspended, pinMask, suspendBehaviour);

	taskEXIT_CRITICAL();
	usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_ACK_OK);
}

static struct PinAssignment *usbCoreGetExistingAssignment(uint8_t pinPacked)
{
	struct PinAssignment *pinAssignment;
	for (int i = 0; i < sizeof(usbCurrentConfiguration.pins.assignments.map) / sizeof(struct PinAssignment); i++)
	{
		pinAssignment = &usbCurrentConfiguration.pins.assignments.map[i];
		if (pinAssignment->pin == pinPacked)
			return pinAssignment;
	}

	return (struct PinAssignment *) 0;
}

bool usbCoreAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (!pinState)
		return false;

	constructUnsuspendedPinState(pinState, args);
	return true;
}

static void constructUnsuspendedPinState(struct PinState *pinState, uint32_t args)
{
	uint8_t direction = (uint8_t) (args & 0x03);
	pinState->bits.tris = (direction & 0x01) ? 0 : 1;
	pinState->bits.ansel = 0;

	uint8_t pinBehaviour = (uint8_t) ((args >> 8) & 0xff);
	uint8_t isOpenCollector = (pinBehaviour >> 2) & 0x01;
	pinState->bits.odc = isOpenCollector ? 1 : 0;

	uint8_t weakPulls = (pinBehaviour >> 0) & 0x03;
	pinState->bits.cnpu = (weakPulls & 0x02) ? 1 : 0;
	pinState->bits.cnpd = (weakPulls == 0x01) ? 1 : 0;

	uint8_t latch = (pinBehaviour >> 3) & 0x01;
	pinState->bits.lat = latch ? 1 : 0;

	uint8_t interruptBehaviour = (uint8_t) ((args >> 16) & 0xff);
	uint8_t isPositiveInterruptEnabled = (interruptBehaviour & 0x04);
	uint8_t isNegativeInterruptEnabled = (interruptBehaviour & 0x02);
	uint8_t isContinuousMode = (interruptBehaviour & 0x01);
	pinState->bits.cnen0 = isPositiveInterruptEnabled ? 1 : 0;
	pinState->bits.cnen1 = isNegativeInterruptEnabled ? 1 : 0;
	pinState->bits.cnenIsContinuous = isContinuousMode ? 1 : 0;

	pinState->rpinr.raw = 0;
	pinState->rpor = 0;
}

static void setPinMaskMapInCurrentConfiguration(uint8_t pinBank, uint32_t pinMask, uint32_t args)
{
	uint8_t pinId = (uint8_t) ((args >> 24) & 0x0f);
	for (int i = 0; i < sizeof(usbCurrentConfiguration.pins.configuration.pinMaskMap) / sizeof(struct PinMaskMap); i++)
	{
		struct PinMaskMap *map = &usbCurrentConfiguration.pins.configuration.pinMaskMap[i];
		if (map->bank == pinBank && map->mask == pinMask)
			map->mask = 0;

		if (i == pinId)
		{
			map->bank = pinBank;
			map->mask = pinMask;	
		}
	}
}

static void unsetPinMaskMapInCurrentConfiguration(uint8_t pinBank, uint32_t pinMask)
{
	for (int i = 0; i < sizeof(usbCurrentConfiguration.pins.configuration.pinMaskMap) / sizeof(struct PinMaskMap); i++)
	{
		struct PinMaskMap *map = &usbCurrentConfiguration.pins.configuration.pinMaskMap[i];
		if (map->bank == pinBank && map->mask == pinMask)
			map->mask = 0;
	}
}

static void setSuspendedPinBankState(struct PinBankState *suspended, const struct PinBankState *unsuspended, uint32_t pinMask, uint8_t suspendBehaviour)
{
	uint8_t weakPulls = (suspendBehaviour >> 0) & 0x03;
	uint8_t isOpenCollector = (suspendBehaviour >> 2) & 0x01;
	uint8_t isRemoteWakeup = (suspendBehaviour >> 5) & 0x01;

	uint8_t suspendPinBehaviour = (suspendBehaviour >> 6) & 0x03;
	switch (suspendPinBehaviour)
	{
		case SUSPEND_BEHAVIOUR_UNCHANGED:
			MASKED_SET(suspended->ansel, unsuspended->ansel, pinMask);
			MASKED_SET(suspended->cnen0, unsuspended->cnen0, pinMask);
			MASKED_SET(suspended->cnen1, unsuspended->cnen1, pinMask);
			MASKED_SET(suspended->cnpd, unsuspended->cnpd, pinMask);
			MASKED_SET(suspended->cnpu, unsuspended->cnpu, pinMask);
			MASKED_SET(suspended->lat, unsuspended->lat, pinMask);
			MASKED_SET(suspended->odc, unsuspended->odc, pinMask);
			MASKED_SET(suspended->tris, unsuspended->tris, pinMask);
			break;

		case SUSPEND_BEHAVIOUR_INPUT:
			suspended->tris |= pinMask;
			MASKED_SET(suspended->odc, isOpenCollector ? pinMask : 0, pinMask);
			MASKED_SET(suspended->cnpu, (weakPulls & 0x02) ? pinMask : 0, pinMask);
			MASKED_SET(suspended->cnpd, (weakPulls == 0x01) ? pinMask : 0, pinMask);
			MASKED_SET(suspended->cnen0, isRemoteWakeup ? pinMask : 0, pinMask);
			MASKED_SET(suspended->cnen1, isRemoteWakeup ? pinMask : 0, pinMask);
			MASKED_SET(suspended->ansel, isRemoteWakeup ? 0 : pinMask, pinMask);
			MASKED_SET(suspended->lat, unsuspended->lat, pinMask);
			break;

		case SUSPEND_BEHAVIOUR_OUTPUT_0:
		case SUSPEND_BEHAVIOUR_OUTPUT_1:
			suspended->tris &= ~pinMask;
			MASKED_SET(suspended->odc, isOpenCollector ? pinMask : 0, pinMask);
			MASKED_SET(suspended->cnpu, (weakPulls & 0x02) ? pinMask : 0, pinMask);
			MASKED_SET(suspended->cnpd, (weakPulls == 0x01) ? pinMask : 0, pinMask);
			suspended->cnen0 &= ~pinMask;
			suspended->cnen1 &= ~pinMask;
			suspended->ansel &= ~pinMask;
			MASKED_SET(suspended->lat, (suspendPinBehaviour == SUSPEND_BEHAVIOUR_OUTPUT_1) ? pinMask : 0, pinMask);
			break;

		default:
			break;
	}
}

int16_t usbCoreGetPinConfigurationReport(uint8_t *report)
{
	uint8_t pinPacked = report[0] & ~CORE_PIN_CONFIGURATION_REPORT_ID_MASK;
	struct PinAssignment *pinAssignment = usbCoreGetExistingAssignment(pinPacked);
	if (!pinAssignment)
		return -1;

	report[1] = 0x00; report[2] = 0x00; report[3] = 0x00; report[4] = 0x00;
	report[5] = 0x00; report[6] = 0x00; report[7] = 0x00; report[8] = 0x00;

	report[9] = pinAssignment->suspendBehaviour;
	report[10] = pinAssignment->interface;
	report[11] = (uint8_t) ((pinAssignment->args >> 0) & 0xff);
	report[12] = (uint8_t) ((pinAssignment->args >> 8) & 0xff);
	report[13] = (uint8_t) ((pinAssignment->args >> 16) & 0xff);
	report[14] = (uint8_t) ((pinAssignment->args >> 24) & 0xff);
	report[15] = (uint8_t) ((pinAssignment->args >> 32) & 0xff);
	report[16] = (uint8_t) ((pinAssignment->args >> 40) & 0xff);
	report[17] = (uint8_t) ((pinAssignment->args >> 48) & 0xff);
	report[18] = (uint8_t) ((pinAssignment->args >> 56) & 0xff);

	return 0;
}
