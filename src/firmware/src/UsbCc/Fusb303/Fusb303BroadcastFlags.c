#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../Fault.h"
#include "../../UsbCc.h"

#include "Fusb303.h"

static void fusb303BroadcastFlagsOnClearInterruptDone(const struct Fusb303Transaction *transaction);
static void fusb303BroadcastFlagsOnDone(void);
static void fusb303BroadcastFlagsOnClearInterrupt1Done(const struct Fusb303Transaction *transaction);
static void fusb303BroadcastFlagsOnReadStatusDone(const struct Fusb303Transaction *transaction);
static void fusb303BroadcastFlagsOnReadStatus1Done(const struct Fusb303Transaction *transaction);
static void fusb303BroadcastFlagsOnReadTypeDone(const struct Fusb303Transaction *transaction);

static uint16_t currentLimitMilliamps = 0;

void fusb303ReadFlagsAndBroadcastFromIsr(BaseType_t *wasHigherPriorityTaskWoken)
{
	static const struct Fusb303Transaction initialTransaction =
	{
		.deviceAddress = FUSB303_I2C_WRITE | FUSB303_I2C_ADDRESS,
		.reg = { .address = FUSB303_REG_INTERRUPT, .value = FUSB303_REG_INTERRUPT_CLEARALL },
		.flags = { .all = 0 },
		.onDone = &fusb303BroadcastFlagsOnClearInterruptDone,
		.context = (void *) ((uint32_t) 0)
	};

	if (xQueueSendToBackFromISR(fusb303Transactions, &initialTransaction, wasHigherPriorityTaskWoken) != pdPASS)
		faultReset(RESET_REASON_FUSB303_QUEUE_FULL);
}

static void fusb303BroadcastFlagsOnClearInterruptDone(const struct Fusb303Transaction *transaction)
{
	if (!transaction->flags.bits.success)
	{
		if (!fusb303TryWriteRegisterWithContext(FUSB303_REG_INTERRUPT, FUSB303_REG_INTERRUPT_CLEARALL, &fusb303BroadcastFlagsOnClearInterruptDone, transaction->context))
			goto retryBroadcastReads;

		return;
	}

	if (!fusb303TryWriteRegisterWithContext(
		FUSB303_REG_INTERRUPT1,
		FUSB303_REG_INTERRUPT1_CLEARALL,
		&fusb303BroadcastFlagsOnClearInterrupt1Done,
		transaction->context))
		goto retryBroadcastReads;

	return;

retryBroadcastReads:
	fusb303BroadcastFlagsOnDone();
}

static void fusb303BroadcastFlagsOnDone(void)
{
	IFS0CLR = _IFS0_INT1IF_MASK;
	IEC0SET = _IEC0_INT1IE_MASK;
}

static void fusb303BroadcastFlagsOnClearInterrupt1Done(const struct Fusb303Transaction *transaction)
{
	if (!transaction->flags.bits.success)
	{
		if (!fusb303TryWriteRegisterWithContext(FUSB303_REG_INTERRUPT1, FUSB303_REG_INTERRUPT1_CLEARALL, &fusb303BroadcastFlagsOnClearInterrupt1Done, transaction->context))
			goto retryBroadcastReads;

		return;
	}

	if (!fusb303TryReadRegisterWithContext(
		FUSB303_REG_STATUS,
		&fusb303BroadcastFlagsOnReadStatusDone,
		transaction->context))
		goto retryBroadcastReads;

	return;

retryBroadcastReads:
	fusb303BroadcastFlagsOnDone();
}

static void fusb303BroadcastFlagsOnReadStatusDone(const struct Fusb303Transaction *transaction)
{
	if (!transaction->flags.bits.success)
	{
		if (!fusb303TryReadRegisterWithContext(FUSB303_REG_STATUS, &fusb303BroadcastFlagsOnReadStatusDone, transaction->context))
			goto retryBroadcastReads;

		return;
	}

	uint8_t status = transaction->reg.value;
	uint8_t orientation = (status >> 4) & 0x03;
	struct UsbCcEvent event =
	{
		.as =
		{
			.flagsChanged =
			{
				.flags =
				{
					.bits =
					{
						.isAttached = (status & FUSB303_REG_STATUS_ATTACH) ? 1 : 0,
						.isVbusVoltageOk = (status & FUSB303_REG_STATUS_VBUSOK) ? 1 : 0,
						.isOrientationCc1A5 = (orientation == 1) ? 1 : 0,
						.isOrientationCc2B5 = (orientation == 2) ? 1 : 0,
						.isOrientationOk = (orientation != 3) ? 1 : 0,
						.currentLimit = (status >> 2)
					}
				}
			}
		}
	};

	if (status & 0x04)
	{
		if (status & 0x02)
			currentLimitMilliamps = 3000;
		else
			currentLimitMilliamps = 1500;
	}
	else
		currentLimitMilliamps = 0;
	
	if (!fusb303TryReadRegisterWithContext(
		FUSB303_REG_STATUS1,
		&fusb303BroadcastFlagsOnReadStatus1Done,
		(void *) ((uint32_t) event.as.flagsChanged.flags.all)))
		goto retryBroadcastReads;

	return;

retryBroadcastReads:
	fusb303BroadcastFlagsOnDone();
}

static void fusb303BroadcastFlagsOnReadStatus1Done(const struct Fusb303Transaction *transaction)
{
	if (!transaction->flags.bits.success)
	{
		if (!fusb303TryReadRegisterWithContext(FUSB303_REG_STATUS1, &fusb303BroadcastFlagsOnReadStatus1Done, transaction->context))
			goto retryBroadcastReads;

		return;
	}

	uint8_t status1 = transaction->reg.value;
	struct UsbCcEvent event =
	{
		.as =
		{
			.flagsChanged =
			{
				.flags = { .all = (uint16_t) ((uint32_t) transaction->context) }
			}
		}
	};

	event.as.flagsChanged.flags.bits.isCcVoltageOk = (status1 & FUSB303_REG_STATUS1_FAULT) ? 0 : 1;

	if (!fusb303TryReadRegisterWithContext(
		FUSB303_REG_TYPE,
		&fusb303BroadcastFlagsOnReadTypeDone,
		(void *) ((uint32_t) event.as.flagsChanged.flags.all)))
		goto retryBroadcastReads;

	return;

retryBroadcastReads:
	fusb303BroadcastFlagsOnDone();
}

static void fusb303BroadcastFlagsOnReadTypeDone(const struct Fusb303Transaction *transaction)
{
	if (!transaction->flags.bits.success)
	{
		if (!fusb303TryReadRegisterWithContext(FUSB303_REG_TYPE, &fusb303BroadcastFlagsOnReadTypeDone, transaction->context))
			goto retryBroadcastReads;

		return;
	}

	uint8_t type = transaction->reg.value;
	struct UsbCcEvent event =
	{
		.header = EVENT_QUEUE_HEADER_INIT(USBCC_MODULE_ID, USBCC_EVENT_FLAGS_CHANGED),
		.as =
		{
			.flagsChanged =
			{
				.flags = { .all = (uint16_t) ((uint32_t) transaction->context) }
			}
		}
	};

	event.as.flagsChanged.flags.bits.isDebugAccessory = (type & FUSB303_REG_TYPE_DEBUGSNK) ? 1 : 0;
	event.as.flagsChanged.flags.bits.isActiveCable = (type & FUSB303_REG_TYPE_ACTIVECABLE) ? 1 : 0;
	event.as.flagsChanged.flags.bits.isAudioAccessory = ((type & FUSB303_REG_TYPE_AUDIO) || (type & FUSB303_REG_TYPE_AUDIOVBUS)) ? 1 : 0;

	if (xQueueSendToBack(fusb303Events, &event, 1) != pdPASS)
		goto retryBroadcastReads;

	fusb303BroadcastFlagsOnDone();
	return;

retryBroadcastReads:
	fusb303BroadcastFlagsOnDone();
}

uint16_t fusb303GetCurrentLimitMilliamps(void)
{
	return currentLimitMilliamps;
}
