#include <xc.h>
#include <stdint.h>

#include "../../FreeRtos.h"

#include "UsbI2cInterface.h"

QueueSetHandle_t usbI2cQueueSet;

void usbI2cInitialise(void)
{
	usbI2cReportsQueue = xQueueCreate(8, sizeof(uint8_t *));
	usbI2cQueueSet = xQueueCreateSet(uxQueueSpacesAvailable(usbI2cReportsQueue) + uxQueueSpacesAvailable(usbI2cEventQueue) + 1);
	xQueueAddToSet(usbI2cReportsQueue, usbI2cQueueSet);
	xQueueAddToSet(usbI2cEventQueue, usbI2cQueueSet);
	xTaskCreate(&usbI2cReportsAndEventsTask, "USB-I2C", RESERVE_STACK_USAGE_BYTES(768), NULL, 3, NULL);
}
