#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"

#include "Spi.h"

struct SpiState spiState;

void spiTask(void *args)
{
	while (true)
	{
		xQueueReceive(spiMasterTransactions, &spiState.master.transaction, portMAX_DELAY);
		spiMasterOnTransactionDequeued(&spiState.master.transaction);
	}
}
