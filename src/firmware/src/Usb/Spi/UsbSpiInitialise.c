#include <xc.h>
#include <stdint.h>

#include "../../FreeRtos.h"

#include "UsbSpiInterface.h"

void usbSpiInitialise(void)
{
	usbSpiReportsQueue = xQueueCreate(8, sizeof(uint8_t *));
	xTaskCreate(&usbSpiReportsTask, "USB-SPI", RESERVE_STACK_USAGE_BYTES(744), NULL, 3, NULL);
}
