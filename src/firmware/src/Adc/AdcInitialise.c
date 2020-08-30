#include <xc.h>
#include <stdint.h>

#include "../Fault.h"
#include "../FreeRtos.h"

#include "Adc.h"

#if configPERIPHERAL_CLOCK_HZ != 24000000
#error The value of the ADCS bits will be wrong if the peripheral clock is not 24MHz
#endif

TaskHandle_t adcTaskHandle;
QueueHandle_t adcTransactions;

void adcInitialise(void)
{
	const int priority = 2;
	IPC8bits.AD1IP = priority;
	IPC8bits.AD1IS = 0;

	xTaskCreate(&adcTask, "ADC", RESERVE_STACK_USAGE_BYTES(544), NULL, priority, &adcTaskHandle);
	adcTransactions = xQueueCreate(4, sizeof(struct AdcTransaction));
}
