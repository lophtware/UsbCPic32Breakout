#include <xc.h>
#include <stdint.h>

#include "../../Pins.h"
#include "../../FreeRtos.h"

#include "UsbCoreInterface.h"

void usbCoreOnPinLatReportReceived(const uint8_t *report)
{
	uint16_t mask = (((uint16_t) report[3]) << 8) | report[2];
	switch (report[1])
	{
		case 0:
			pinsMaskedLatClear(mask);
			break;

		case 1:
			pinsMaskedLatSet(mask);
			break;

		case 2:
			pinsMaskedLatToggle(mask);
			break;

		case 3:
			pinsMaskedLatLoad(mask);
			break;
	}

	taskENTER_CRITICAL();
	uint32_t lats[] = {LATA, LATB, LATC};
	uint32_t ports[] = {PORTA, PORTB, PORTC};
	taskEXIT_CRITICAL();

	uint16_t latMask = 0, portMask = 0;
	for (int i = 0; i < sizeof(usbCurrentConfiguration.pins.configuration.pinMaskMap) / sizeof(struct PinMaskMap); i++)
	{
		struct PinMaskMap *map = &usbCurrentConfiguration.pins.configuration.pinMaskMap[i];
		if (lats[map->bank] & map->mask)
			latMask |= 1 << i;

		if (ports[map->bank] & map->mask)
			portMask |= 1 << i;
	}

	struct UsbSmallReport statusReport =
	{
		.as =
		{
			.fields =
			{
				.payload =
				{
					CORE_PIN_STATUS_REPORT_ID,
					PIN_STATUS_REPORT_FLAG_RESPONSE,
					(uint8_t) ((latMask >> 0) & 0xff),
					(uint8_t) ((latMask >> 8) & 0xff),
					(uint8_t) ((portMask >> 0) & 0xff),
					(uint8_t) ((portMask >> 8) & 0xff),
					0x00
				},
				.flags = { .endpoint = CORE_EP_ID, .count = 6 }
			}
		}
	};

	while (!usbSmallReportSend(statusReport))
	{
		vTaskDelay(1);
	}
}

int16_t usbCoreGetPinStatusReport(uint8_t *report)
{
	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	uint32_t lats[] = {LATA, LATB, LATC};
	uint32_t ports[] = {PORTA, PORTB, PORTC};
	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	uint16_t latMask = 0, portMask = 0;
	for (int i = 0; i < sizeof(usbCurrentConfiguration.pins.configuration.pinMaskMap) / sizeof(struct PinMaskMap); i++)
	{
		struct PinMaskMap *map = &usbCurrentConfiguration.pins.configuration.pinMaskMap[i];
		if (lats[map->bank] & map->mask)
			latMask |= 1 << i;

		if (ports[map->bank] & map->mask)
			portMask |= 1 << i;
	}

	report[0] = CORE_PIN_STATUS_REPORT_ID;
	report[1] = 0x00;
	report[2] = (uint8_t) ((latMask >> 0) & 0xff);
	report[3] = (uint8_t) ((latMask >> 8) & 0xff);
	report[4] = (uint8_t) ((portMask >> 0) & 0xff);
	report[5] = (uint8_t) ((portMask >> 8) & 0xff);
	return 0;
}
