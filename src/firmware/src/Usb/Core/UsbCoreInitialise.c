#include <xc.h>
#include <stdint.h>

#include "../../FreeRtos.h"
#include "../../Pins.h"

#include "UsbCoreInterface.h"

void usbCoreInitialise(void)
{
	usbCoreReportsQueue = xQueueCreate(8, sizeof(uint8_t *));
	xTaskCreate(&usbCoreReportsTask, "USB-CORE", RESERVE_STACK_USAGE_BYTES(800), NULL, 3, NULL);

	TaskHandle_t pinsTask;
	xTaskCreate(&usbCorePinsTask, "USB-PINS", RESERVE_STACK_USAGE_BYTES(480), NULL, 3, &pinsTask);
	pinsOnChangedNotify(pinsTask);
}
