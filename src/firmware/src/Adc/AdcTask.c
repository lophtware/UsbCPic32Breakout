#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"

#include "Adc.h"

void adcTask(void *args)
{
	static struct AdcTransaction transaction;
	while (true)
	{
		xQueueReceive(adcTransactions, &transaction, portMAX_DELAY);
		adcOnTransactionDequeued(&transaction);
	}
}
