#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../Fault.h"
#include "../../FreeRtos.h"

#include "UsbCcAdc.h"

TaskHandle_t usbCcAdcTaskHandle;
QueueHandle_t usbCcAdcEvents;
uint16_t usbCcAdcCurrentLimitMilliamps = 0;

void usbCcAdcInitialise(QueueHandle_t eventQueue)
{
	if (!eventQueue)
		faultUnrecoverableInitialisationError();

	usbCcAdcEvents = eventQueue;
	xTaskCreate(&usbCcAdcTask, "USBCCAD", RESERVE_STACK_USAGE_BYTES(608), NULL, 2, &usbCcAdcTaskHandle);
}

bool usbCcAdcIsInitialised(void)
{
	return usbCcAdcCurrentLimitMilliamps > 0;
}

uint16_t usbCcAdcGetCurrentLimitMilliamps(void)
{
	return usbCcAdcCurrentLimitMilliamps > 0 ? usbCcAdcCurrentLimitMilliamps : 100;
}
