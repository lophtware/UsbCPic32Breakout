#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "UsbCoreInterface.h"

#include "../../FreeRtos.h"
#include "../../Pins.h"

void usbCorePinsTask(void *args)
{
	while (true)
	{
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

		while (usbIsInEndpointBusy(CORE_EP_ID))
		{
			vTaskDelay(1);
		}

		uint8_t *report = (uint8_t *) usb_get_in_buffer(CORE_EP_ID);
		uint32_t firstTimestamp = pinsGetLastOnChangedTimestamp();

		taskENTER_CRITICAL();
		uint32_t secondTimestamp = _CP0_GET_COUNT();
		uint32_t cnfs[] = {CNFA, CNFB, CNFC};
		uint32_t ports[] = {PORTA, PORTB, PORTC};
		uint32_t lats[] = {LATA, LATB, LATC};

		CNFACLR = cnfs[0];
		CNFBCLR = cnfs[1];
		CNFCCLR = cnfs[2];
		bool stableDuringSnapshot = (CNFA | CNFB | CNFC) == 0;
		taskEXIT_CRITICAL();

		uint16_t latMask = 0, portMask = 0, cnfMask = 0;
		for (int i = 0; i < sizeof(usbCurrentConfiguration.pins.configuration.pinMaskMap) / sizeof(struct PinMaskMap); i++)
		{
			struct PinMaskMap *map = &usbCurrentConfiguration.pins.configuration.pinMaskMap[i];
			if (lats[map->bank] & map->mask)
				latMask |= 1 << i;

			if (ports[map->bank] & map->mask)
				portMask |= 1 << i;

			if (cnfs[map->bank] & map->mask)
				cnfMask |= 1 << i;
		}

		report[0] = CORE_PINS_CHANGED_REPORT_ID;
		report[1] = stableDuringSnapshot ? 0x02 : 0x00;
		report[2] = ((uint8_t) (latMask >> 0) & 0xff);
		report[3] = ((uint8_t) (latMask >> 8) & 0xff);
		report[4] = ((uint8_t) (portMask >> 0) & 0xff);
		report[5] = ((uint8_t) (portMask >> 8) & 0xff);
		report[6] = ((uint8_t) (cnfMask >> 0) & 0xff);
		report[7] = ((uint8_t) (cnfMask >> 8) & 0xff);
		report[8] = (uint8_t) ((firstTimestamp >> 0) & 0xff);
		report[9] = (uint8_t) ((firstTimestamp >> 8) & 0xff);
		report[10] = (uint8_t) ((firstTimestamp >> 16) & 0xff);
		report[11] = (uint8_t) ((firstTimestamp >> 24) & 0xff);
		report[12] = (uint8_t) ((secondTimestamp >> 0) & 0xff);
		report[13] = (uint8_t) ((secondTimestamp >> 8) & 0xff);
		report[14] = (uint8_t) ((secondTimestamp >> 16) & 0xff);
		report[15] = (uint8_t) ((secondTimestamp >> 24) & 0xff);
		usb_send_in_buffer(CORE_EP_ID, 16);

		pinsChangedReset();
	}
}
