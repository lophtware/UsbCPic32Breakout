#include <xc.h>
#include <stdint.h>

#include "../../UnlockKey.h"

#include "UsbUsbInterface.h"

#define UPDATE_DESCRIPTOR_FLAG 0x80
#define UPDATE_CHARGER_TEST_FLAG 0x80
#define UPDATE_CHARGER_LIMIT_FLAG 0x40
#define CHARGER_TEST_FLAG 0x01

__attribute__((packed))
struct UsbPowerConfigurationReport
{
	uint8_t reportId;

	struct
	{
		uint8_t flags;
		uint8_t low;
		uint8_t high;
	} descriptor;

	struct
	{
		uint8_t flags;
		uint8_t low;
		uint8_t high;
	} dedicatedCharger;
};

void usbUsbOnPowerConfigurationReportReceived(const uint8_t *report)
{
	const struct UsbPowerConfigurationReport *in = (const struct UsbPowerConfigurationReport *) report;

	uint16_t enumeratedCurrentLimitMilliamps = ((uint16_t) in->descriptor.high << 8) | in->descriptor.low;
	if (in->descriptor.flags & UPDATE_DESCRIPTOR_FLAG)
	{
		if (enumeratedCurrentLimitMilliamps == 0 || enumeratedCurrentLimitMilliamps > 500)
		{
			usbSendAcknowledgementReport(USB_EP_ID, USB_POWER_CONFIGURATION_REPORT_ID, REPORT_NACK_OUT_OF_BOUNDS);
			return;
		}
	}

	uint16_t chargerCurrentLimitMilliamps = ((uint16_t) in->dedicatedCharger.high << 8) | in->dedicatedCharger.low;
	if (in->dedicatedCharger.flags & UPDATE_CHARGER_LIMIT_FLAG)
	{
		if (chargerCurrentLimitMilliamps == 0 || chargerCurrentLimitMilliamps > 1500)
		{
			usbSendAcknowledgementReport(USB_EP_ID, USB_POWER_CONFIGURATION_REPORT_ID, REPORT_NACK_OUT_OF_BOUNDS);
			return;
		}
	}

	taskENTER_CRITICAL();

	if (in->descriptor.flags & UPDATE_DESCRIPTOR_FLAG)
		usbCurrentConfiguration.currentLimitMilliamps = enumeratedCurrentLimitMilliamps;

	if (in->dedicatedCharger.flags & UPDATE_CHARGER_LIMIT_FLAG)
		usbCurrentConfiguration.dedicatedChargerAssumedCurrentLimitMilliamps = chargerCurrentLimitMilliamps;

	if (in->dedicatedCharger.flags & UPDATE_CHARGER_TEST_FLAG)
		usbCurrentConfiguration.isUsbShortedDataLineTestEnabled = (in->dedicatedCharger.flags & CHARGER_TEST_FLAG) ? true : false;

	taskEXIT_CRITICAL();

	usbSendAcknowledgementReport(USB_EP_ID, USB_POWER_CONFIGURATION_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbUsbGetPowerConfigurationReport(uint8_t *report)
{
	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	uint16_t enumeratedCurrentLimitMilliamps = usbCurrentConfiguration.currentLimitMilliamps;
	uint16_t chargerCurrentLimitMilliamps = usbCurrentConfiguration.dedicatedChargerAssumedCurrentLimitMilliamps;
	bool isChargerTestEnabled = usbCurrentConfiguration.isUsbShortedDataLineTestEnabled;
	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	struct UsbPowerConfigurationReport *out = (struct UsbPowerConfigurationReport *) report;
	out->reportId = USB_POWER_CONFIGURATION_REPORT_ID;
	out->descriptor.flags = 0;
	out->descriptor.low = (uint8_t) ((enumeratedCurrentLimitMilliamps >> 0) & 0xff);
	out->descriptor.high = (uint8_t) ((enumeratedCurrentLimitMilliamps >> 8) & 0xff);
	out->dedicatedCharger.flags = isChargerTestEnabled ? CHARGER_TEST_FLAG : 0x00;
	out->dedicatedCharger.low = (uint8_t) ((chargerCurrentLimitMilliamps >> 0) & 0xff);
	out->dedicatedCharger.high = (uint8_t) ((chargerCurrentLimitMilliamps >> 8) & 0xff);
	return 0;
}
