#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../Delay.h"
#include "../Dma.h"
#include "../FreeRtos.h"

#include "Spi.h"

#define SPICON_INTERRUPT_WHEN_TX_NOT_FULL (3 << _SPI2CON_STXISEL_POSITION)
#define SPICON_INTERRUPT_WHEN_RX_NOT_EMPTY (1 << _SPI2CON_SRXISEL_POSITION)
#define SPICON_MASK (_SPI2CON_MSTEN_MASK | _SPI2CON_ENHBUF_MASK | SPICON_INTERRUPT_WHEN_TX_NOT_FULL | SPICON_INTERRUPT_WHEN_RX_NOT_EMPTY)
#define SPICON2_MASK (_SPI2CON2_IGNROV_MASK | _SPI2CON2_IGNTUR_MASK)

static inline uint32_t dmaCellSizeFromSpiSettings(void);

bool spiMasterSend(const struct SpiMasterTransaction *transaction)
{
	if (!transaction ||
		!transaction->onDone ||
		(transaction->inChunkLength > 0 && transaction->inLength > 0 && !transaction->onChunkReceived) ||
		(transaction->outLength > 0 && !transaction->out) ||
		(transaction->inLength > 0 && !transaction->in) ||
		(transaction->inLength == 0 && transaction->outLength == 0))
	{
		return false;
	}

	if (xQueueSendToBack(spiMasterTransactions, transaction, pdMS_TO_TICKS(100)) == pdPASS)
		return true;

	return false;
}

void spiMasterOnTransactionDequeued(struct SpiMasterTransaction *transaction)
{
	static struct DmaTransaction dmaRxTransaction =
	{
		.source = { .ptr = (const void *) &SPI2BUF },
		.cellSize = 1,
		.priority = DMA_CHANNEL_PRIORITY_HIGH,
		.transferIrq = _SPI2_RX_VECTOR,
		.abortIrq = DMA_IRQ_UNUSED,
		.isContinuous = false,
		.needsStarting = false
	};

	static struct DmaTransaction dmaTxTransaction =
	{
		.destination = { .ptr = (void *) &SPI2BUF },
		.cellSize = 1,
		.priority = DMA_CHANNEL_PRIORITY_HIGH,
		.transferIrq = _SPI2_TX_VECTOR,
		.abortIrq = DMA_IRQ_UNUSED,
		.isContinuous = false,
		.needsStarting = true
	};

	if (!transaction)
		return;

	uint8_t slaveAddress = transaction->flags.bits.slaveAddress;
	if (slaveAddress >= SPI_MAXIMUM_SLAVES)
		return;

	spiState.master.flags.asBits.isBusy = 1;
	transaction->statusFlags.all = 0;
	SPI2CON = SPICON_MASK | spiState.configuration.slaves[slaveAddress].registers.con;
	SPI2CON2 = SPICON2_MASK | spiState.configuration.slaves[slaveAddress].registers.con2;
	SPI2BRG = spiState.configuration.slaves[slaveAddress].registers.brg;

	SPI2CONSET = _SPI2CON_ON_MASK;
	spiSlavesSelect(slaveAddress);

	dmaRxTransaction.cellSize = dmaCellSizeFromSpiSettings();
	dmaRxTransaction.source.length = dmaRxTransaction.cellSize;

	dmaTxTransaction.cellSize = dmaRxTransaction.cellSize;
	dmaTxTransaction.destination.length = dmaRxTransaction.cellSize;

	do
	{
		dmaRxTransaction.destination.ptr = transaction->in;
		dmaRxTransaction.destination.length = transaction->inLength;

		uint8_t isReceiving = dmaRxTransaction.destination.ptr && dmaRxTransaction.destination.length;
		if (isReceiving)
		{
			while (!dmaRxTransaction.channel && !(dmaRxTransaction.channel = dmaChannelTryAllocate()))
				vTaskDelay(1);

			dmaTransactionStart(&dmaRxTransaction);
		}

		uint8_t isTransmitting = transaction->out && transaction->outLength;
		if (!isTransmitting)
		{
			dmaTxTransaction.source.ptr = transaction->in;
			dmaTxTransaction.source.length = transaction->inLength;
			SPI2CONSET = _SPI2CON_DISSDO_MASK;
		}
		else
		{
			dmaTxTransaction.source.ptr = transaction->out;
			dmaTxTransaction.source.length = transaction->outLength;
			SPI2CONCLR = _SPI2CON_DISSDO_MASK;
		}

		while (!dmaTxTransaction.channel && !(dmaTxTransaction.channel = dmaChannelTryAllocate()))
			vTaskDelay(1);

		dmaTransactionStart(&dmaTxTransaction);

		uint32_t startTime = now();
		const uint32_t timeout = 10 * TICKS_PER_SECOND;

		const uint8_t defaultPollIntervalTicks = 1;
		uint16_t bytesReceived = 0;
		spiState.master.inChunkArgs.chunkNumber = 0;
		spiState.master.inChunkArgs.pollIntervalTicks = defaultPollIntervalTicks;
		spiState.master.inChunkArgs.pollTaskHandle = spiTaskHandle;

		while (SPI2CONbits.ON)
		{
			ulTaskNotifyTake(pdTRUE, spiState.master.inChunkArgs.pollIntervalTicks);
			if (isReceiving)
			{
				uint16_t bytesReceivedSince = dmaChannelBytesTransferred(dmaRxTransaction.channel) - bytesReceived;
				if (
					(transaction->inChunkLength > 0 && bytesReceivedSince >= transaction->inChunkLength) ||
					(bytesReceived + bytesReceivedSince) >= transaction->inLength)
				{
					spiState.master.inChunkArgs.consumed = 0;
					spiState.master.inChunkArgs.length = bytesReceivedSince;
					spiState.master.inChunkArgs.pollIntervalTicks = defaultPollIntervalTicks;
					transaction->onChunkReceived(transaction, &spiState.master.inChunkArgs);
					if (spiState.master.inChunkArgs.consumed != 0)
					{
						if (spiState.master.inChunkArgs.consumed <= bytesReceivedSince)
							bytesReceived += spiState.master.inChunkArgs.consumed;
						else
							bytesReceived += bytesReceivedSince;

						spiState.master.inChunkArgs.chunkNumber++;
					}

					if (bytesReceived >= transaction->inLength)
						isReceiving = false;
				}
			}

			if (bytesReceived >= transaction->inLength && bytesReceived >= transaction->outLength)
				break;

			if (!dmaChannelIsBusy(dmaTxTransaction.channel))
			{
				if (bytesReceived >= transaction->inLength)
					break;

				while (SPI2STATbits.SPIBUSY)
					;;

				SPI2CONSET = _SPI2CON_DISSDO_MASK;

				if (dmaTxTransaction.source.length < dmaRxTransaction.destination.length)
				{
					dmaTxTransaction.source.ptr = dmaRxTransaction.destination.ptr;
					dmaTxTransaction.source.length = dmaRxTransaction.destination.length - dmaTxTransaction.source.length;
					dmaTransactionStart(&dmaTxTransaction);
				}
			}

			if (elapsedSince(startTime) > timeout)
			{
				transaction->statusFlags.bits.timeout = 1;
				break;
			}
		}

		if (transaction->statusFlags.bits.timeout || !transaction->onRepeat || !SPI2CONbits.ON)
			break;

	} while (transaction->onRepeat(transaction));

	if (dmaChannelIsBusy(dmaRxTransaction.channel))
		dmaTransactionAbort(&dmaRxTransaction);

	dmaChannelFree(dmaRxTransaction.channel);
	dmaRxTransaction.channel = 0;

	if (dmaChannelIsBusy(dmaTxTransaction.channel))
		dmaTransactionAbort(&dmaTxTransaction);

	dmaChannelFree(dmaTxTransaction.channel);
	dmaTxTransaction.channel = 0;

	while (SPI2STATbits.SPIBUSY)
		;;

	spiSlavesDeselect(slaveAddress);

	SPI2CONCLR = _SPI2CON_ON_MASK;

	spiState.master.flags.asBits.isBusy = 0;

	transaction->statusFlags.bits.done = 1;
	transaction->onDone(transaction);
}

static inline uint32_t dmaCellSizeFromSpiSettings(void)
{
	if (SPI2CON2bits.AUDEN)
	{
		if (SPI2CONbits.MODE32)
		{
			if (SPI2CONbits.MODE16)
				return 3;
			else
				return 4;
		}

		return 2;
	}

	if (SPI2CONbits.MODE32)
		return 4;
	else if (SPI2CONbits.MODE16)
		return 2;

	return 1;
}

bool spiMasterIsIdleFromIsr(void)
{
	return (spiState.master.flags.asBits.isBusy || uxQueueMessagesWaitingFromISR(spiMasterTransactions) > 0)
		? false
		: true;
}
