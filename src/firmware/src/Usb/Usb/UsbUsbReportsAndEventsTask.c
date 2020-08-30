#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "UsbUsbInterface.h"

#include "../../EventQueueHeader.h"
#include "../../UsbCc.h"
#include "../../FreeRtos.h"

#define CHECKED_FOR_DEDICATED_CHARGING_PORT_MASK (1 << 9)
#define DEDICATED_CHARGING_PORT_MASK (1 << 10)
#define SUSPENDED_MASK (1 << 11)
#define ENUMERATED_MASK (1 << 12)
#define PROTOCOL_ERROR_MASK (1 << 13)
#define CURRENT_CAPABILITY_MASK (1 << 14)

#define USBCC_CABLE_FLAGS_MASK 0x01ff

struct KnownEvents
{
	union
	{
		struct EventQueueHeader header;
		struct UsbCcEvent cc;
		struct UsbEvent usb;
	} as;
};

static struct
{
	uint16_t ccCurrentLimitMilliamps;
	uint16_t dedicatedChargerCurrentLimitMilliamps;
	uint16_t enumeratedCurrentLimitMilliamps;
	uint16_t applicableCurrentLimitMilliamps;
} usbPowerCapability;

static uint16_t usbUsbStatusFlags;

static inline uint16_t max3(uint16_t a, uint16_t b, uint16_t c);
static void onUsbFlaggableEvent(uint16_t cableFlags);
static struct UsbUsbAssignedPin *assignedPinEntryFor(const struct Pin pin);

QueueHandle_t usbFlagsEventQueue;
QueueHandle_t usbUsbReportsQueue;

void usbUsbReportsAndEventsTask(void *args)
{
	usbInitialiseAfterUsbCc();

	static struct KnownEvents event;
	static uint16_t flagsNonAtomic;
	flagsNonAtomic = usbWasDedicatedChargingPortTestPerformed() ? CHECKED_FOR_DEDICATED_CHARGING_PORT_MASK : 0;
	while (true)
	{
		static QueueSetMemberHandle_t queue;
		queue = xQueueSelectFromSet(usbUsbQueueSet, portMAX_DELAY);
		if (queue == usbUsbReportsQueue)
		{
			static const uint8_t *report;
			xQueueReceive(queue, &report, 0);
			if (!report)
				continue;

			switch (report[0])
			{
				case USB_CONFIGURATION_NAME_DESCRIPTOR_REPORT_ID:
					usbUsbOnConfigurationNameDescriptorReportReceived(report);
					break;

				case USB_POWER_CONFIGURATION_REPORT_ID:
					usbUsbOnPowerConfigurationReportReceived(report);
					break;

				default:
					usbSendAcknowledgementReport(USB_EP_ID, report[0], REPORT_NACK_UNKNOWN_ID);
					break;
			}
		}
		else
		{
			xQueueReceive(usbFlagsEventQueue, &event, 0);
			switch (event.as.header.as.raw)
			{
				case EVENT_QUEUE_HEADER_WORD_FOR(USBCC_MODULE_ID, USBCC_EVENT_FLAGS_CHANGED):
					flagsNonAtomic &= ~USBCC_CABLE_FLAGS_MASK;
					flagsNonAtomic |= event.as.cc.as.flagsChanged.flags.all & USBCC_CABLE_FLAGS_MASK;
					usbPowerCapability.ccCurrentLimitMilliamps = usbCcGetCurrentLimitMilliamps();
					break;

				case EVENT_QUEUE_HEADER_WORD_FOR(USB_MODULE_ID, USB_EVENT_CONFIGURATION_CHANGED):
					usbConfigurationApply(&event.as.usb);
					if (usb_is_configured())
					{
						flagsNonAtomic |= ENUMERATED_MASK;
						usbPowerCapability.enumeratedCurrentLimitMilliamps = usbCurrentConfiguration.currentLimitMilliamps;
					}
					else
					{
						flagsNonAtomic &= ~ENUMERATED_MASK;
						usbPowerCapability.enumeratedCurrentLimitMilliamps = 100;
					}
					break;

				default:
					break;
			}

			flagsNonAtomic &= ~PROTOCOL_ERROR_MASK;
			flagsNonAtomic |= (U1EIR != 0 || usbGetHaltedEndpointsAsMask() != 0) ? PROTOCOL_ERROR_MASK : 0;

			if (usbIsAttachedToDedicatedChargingPort())
			{
				flagsNonAtomic |= DEDICATED_CHARGING_PORT_MASK;
				usbPowerCapability.dedicatedChargerCurrentLimitMilliamps = usbCurrentConfiguration.dedicatedChargerAssumedCurrentLimitMilliamps;
			}
			else
			{
				flagsNonAtomic &= ~DEDICATED_CHARGING_PORT_MASK;
				usbPowerCapability.dedicatedChargerCurrentLimitMilliamps = 0;		
			}

			usbPowerCapability.applicableCurrentLimitMilliamps = max3(
				usbPowerCapability.enumeratedCurrentLimitMilliamps,
				usbPowerCapability.dedicatedChargerCurrentLimitMilliamps,
				usbPowerCapability.ccCurrentLimitMilliamps);

			onUsbFlaggableEvent(flagsNonAtomic);
		}
	}
}

static inline uint16_t max3(uint16_t a, uint16_t b, uint16_t c)
{
	uint16_t d = (a > b) ? a : b;
	return c > d ? c : d;
}

static void onUsbFlaggableEvent(uint16_t flags)
{
	vTaskSuspendAll();
	for (int i = 0; i < PINS_NUMBER_CONFIGURABLE; i++)
	{
		const struct UsbUsbAssignedPin *assignment = &usbUsbAssignedPins[i];
		if (assignment->flags.bits.isStatusFlagOutput)
		{
			if (usbPowerCapability.applicableCurrentLimitMilliamps >= assignment->currentMilliamps)
				flags |= CURRENT_CAPABILITY_MASK;
			else
				flags &= ~CURRENT_CAPABILITY_MASK;

			uint16_t masked = (flags ^ assignment->maskXor) & assignment->mask;
			if ((assignment->flags.bits.isAnd && masked == assignment->mask) || (!assignment->flags.bits.isAnd && masked != 0))
			{
				if (assignment->flags.bits.isActiveHigh)
					pinsLatSet(assignment->pin);
				else
					pinsLatClear(assignment->pin);
			}
			else
			{
				if (assignment->flags.bits.isActiveHigh)
					pinsLatClear(assignment->pin);
				else
					pinsLatSet(assignment->pin);
			}
		}
	}

	usbUsbStatusFlags = flags & ~CURRENT_CAPABILITY_MASK;
	xTaskResumeAll();
}

uint32_t usbUsbGetStatusFlags(void)
{
	return usbUsbStatusFlags;
}

uint32_t usbUsbGetCurrentLimitMilliamps(void)
{
	return usbPowerCapability.applicableCurrentLimitMilliamps;
}

bool usbUsbAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	struct UsbUsbAssignedPin *assignedPin = assignedPinEntryFor(pin);
	if (!pinState || !assignedPin)
		return false;

	uint8_t function = (((uint8_t) (args >> 0)) & 0xff);
	if (function != 0x08 && function != 0x09)
		return false;

	uint8_t pinBehaviour = (((uint8_t) (args >> 8)) & 0xff);

	assignedPin->flags.all = 0;
	assignedPin->flags.bits.isStatusFlagOutput = 1;
	assignedPin->flags.bits.isActiveHigh = (pinBehaviour & 0x08) ? 1 : 0;
	assignedPin->flags.bits.isAnd = (function & 0x01) ? 1 : 0;
	assignedPin->mask = (((uint16_t) (args >> 16)) & 0x7fff);
	assignedPin->maskXor = (((uint16_t) (args >> 32)) & 0x7fff);
	assignedPin->currentMilliamps = (((uint16_t) (args >> 48)) & 0x03ff) * 2;

	pinState->bits.ansel = 0;
	pinState->bits.cncon = 0;
	pinState->bits.cnen0 = 0;
	pinState->bits.cnen1 = 0;
	pinState->bits.cnenIsContinuous = 0;
	pinState->bits.cnpd = ((pinBehaviour & 0x03) == 1) ? 1 : 0;
	pinState->bits.cnpu = (pinBehaviour & 0x02) ? 1 : 0;
	pinState->bits.lat = assignedPin->flags.bits.isActiveHigh ? 0 : 1;
	pinState->bits.odc = (pinBehaviour & 0x04) ? 1 : 0;
	pinState->bits.port = 0;
	pinState->bits.tris = 0;
	pinState->rpinr.raw = 0;
	pinState->rpor = 0;

	static const struct UsbEvent event =
	{
		.header = EVENT_QUEUE_HEADER_INIT(USB_MODULE_ID, USB_EVENT_FLAGS_EVALUATION_CHANGED)
	};

	return xQueueSendToBack(usbFlagsEventQueue, &event, 1) == pdPASS;
}

static struct UsbUsbAssignedPin *assignedPinEntryFor(const struct Pin pin)
{
	for (int i = 0; i < PINS_NUMBER_CONFIGURABLE; i++)
	{
		if (usbUsbAssignedPins[i].pin.bank == pin.bank && usbUsbAssignedPins[i].pin.index == pin.index)
			return &usbUsbAssignedPins[i];
	}

	return (struct UsbUsbAssignedPin *) 0;
}

void usbUsbUnassignPin(const struct Pin pin)
{
	struct UsbUsbAssignedPin *assignedPin = assignedPinEntryFor(pin);
	if (!assignedPin)
		return;

	assignedPin->flags.all = 0;
}
