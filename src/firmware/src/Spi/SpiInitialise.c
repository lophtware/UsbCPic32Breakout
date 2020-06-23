#include <xc.h>
#include <stdint.h>

#include "../Fault.h"
#include "../FreeRtos.h"

#include "Spi.h"

#if configPERIPHERAL_CLOCK_HZ != 24000000
#error The value of the SPI2BRG register will be wrong if the peripheral clock is not 24MHz
#endif

TaskHandle_t spiTaskHandle;
QueueHandle_t spiMasterTransactions;

void spiInitialise(void)
{
	const int priority = 3;
	IPC11bits.SPI2TXIP = priority;
	IPC11bits.SPI2TXIS = 0;
	IPC11bits.SPI2RXIP = priority;
	IPC11bits.SPI2RXIS = 0;
	IPC11bits.SPI2EIP = priority;
	IPC11bits.SPI2EIS = 0;

	xTaskCreate(&spiTask, "SPI", RESERVE_STACK_USAGE_BYTES(544), NULL, priority, &spiTaskHandle);
	spiMasterTransactions = xQueueCreate(4, sizeof(struct SpiMasterTransaction));
}
