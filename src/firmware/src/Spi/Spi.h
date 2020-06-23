#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_SPI_SPI_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_SPI_SPI_H
#include <stdint.h>
#include "../Pins.h"
#include "../Spi.h"

#define SPI_MAXIMUM_SLAVES (sizeof(spiState.configuration.slaves) / sizeof(struct SpiSlaveConfiguration))

struct SpiMasterTransactionState
{
	struct SpiMasterTransaction transaction;

	const uint8_t *outPtr;

	uint8_t *inPtr;
	struct SpiMasterOnChunkReceivedEventArgs inChunkArgs;

	union
	{
		uint8_t asRaw;
		struct
		{
			unsigned int isBusy : 1;
			unsigned int : 7;
		} asBits;
	} flags;
};

struct SpiState
{
	struct SpiMasterTransactionState master;
	struct SpiConfiguration configuration;
};

extern TaskHandle_t spiTaskHandle;
extern QueueHandle_t spiMasterTransactions;
extern struct SpiState spiState;

extern void spiTask(void *args);

extern void spiSlavesSelect(uint8_t index);
extern void spiSlavesDeselect(uint8_t index);

extern void spiMasterOnTransactionDequeued(struct SpiMasterTransaction *transaction);

#endif
