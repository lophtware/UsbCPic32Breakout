#include <xc.h>
#include <stdint.h>

#include "../../Pins.h"
#include "../../FreeRtos.h"

#include "UsbCoreInterface.h"

void usbCoreOnPinsChangedResetReportReceived(const uint8_t *report)
{
	uint16_t mask = (((uint16_t) report[2]) << 8) | report[3];

	taskENTER_CRITICAL();
	uint32_t lats[] = {LATA, LATB, LATC};
	uint32_t ports[] = {PORTA, PORTB, PORTC};
	taskEXIT_CRITICAL();

	uint16_t latMask = 0, portMask = 0;
	uint16_t aMask = 0, bMask = 0, cMask = 0;
	for (int i = 0; i < sizeof(usbCurrentConfiguration.pins.configuration.pinMaskMap) / sizeof(struct PinMaskMap); i++)
	{
		struct PinMaskMap *map = &usbCurrentConfiguration.pins.configuration.pinMaskMap[i];
		if (lats[map->bank] & map->mask)
			latMask |= 1 << i;

		if (ports[map->bank] & map->mask)
			portMask |= 1 << i;

		if (mask & (1 << i))
		{
			if (map->bank == 0)
				aMask |= map->mask;
			else if (map->bank == 1)
				bMask |= map->mask;
			else if (map->bank == 2)
				cMask |= map->mask;
		}
	}

	pinsChangedResetNonContinuous(aMask, bMask, cMask);

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
