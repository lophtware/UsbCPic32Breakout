#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"
#include "../Dma.h"

#include "Adc.h"

#define AD1CON1_AUTO_CONVERT_MASK (7 << _AD1CON1_SSRC_POSITION)

bool adcTransactionTryStart(const struct AdcTransaction *transaction)
{
	if (!transaction ||
		!transaction->sampleBuffer ||
		!transaction->numberOfSamples ||
		!transaction->onDone ||
		transaction->channel > ADC_CHANNEL_AVDD ||
		(transaction->channel > ADC_CHANNEL_AN11 && transaction->channel < ADC_CHANNEL_VDDCORE) ||
		transaction->tadDivision < ADC_TAD_DIVISION_FASTEST ||
		transaction->tadDivision > ADC_TAD_DIVISION_SLOWEST ||
		transaction->tadBetweenSamples < ADC_TAD_BETWEEN_FASTEST ||
		transaction->tadBetweenSamples > ADC_TAD_BETWEEN_SLOWEST)
	{
		return false;
	}

	return xQueueSendToBack(adcTransactions, transaction, pdMS_TO_TICKS(100)) == pdPASS;
}

void adcOnTransactionDequeued(struct AdcTransaction *transaction)
{
	if (!transaction)
		return;

	AD1CON1 =
		(transaction->flags.bits.sampleFormat << _AD1CON1_FORM_POSITION) |
		AD1CON1_AUTO_CONVERT_MASK |
		(transaction->flags.bits.is12Bit ? _AD1CON1_MODE12_MASK : 0) |
		_AD1CON1_ASAM_MASK;

	AD1CON2 = 0;

	AD1CON3 =
		(transaction->tadBetweenSamples << _AD1CON3_SAMC_POSITION) |
		(transaction->tadDivision << _AD1CON3_ADCS_POSITION);

	AD1CON5 = 0;

	AD1CHS = transaction->channel << _AD1CHS_CH0SA_POSITION;

	static struct DmaTransaction dma =
	{
		.source = { .ptr = (const void *) &ADC1BUF0, .length = 2 },
		.channel = 0,
		.cellSize = 2,
		.priority = DMA_CHANNEL_PRIORITY_LOW,
		.transferIrq = _ADC_VECTOR,
		.abortIrq = DMA_IRQ_UNUSED,
		.isContinuous = false,
		.needsStarting = false
	};

	while (!dma.channel && !(dma.channel = dmaChannelTryAllocate()))
		vTaskDelay(1);

	AD1CON1SET = _AD1CON1_ON_MASK;
	AD1CON1SET = _AD1CON1_SAMP_MASK;

	do
	{
		dma.destination.ptr = transaction->sampleBuffer;
		dma.destination.length = transaction->numberOfSamples * 2;

		dmaTransactionStart(&dma);
		while (dmaChannelIsBusy(dma.channel))
		{
			if (transaction->numberOfSamples > 400)
				vTaskDelay(1);
		}
	} while (transaction->onDone(transaction));

	AD1CON1CLR = _AD1CON1_ON_MASK;

	dmaChannelFree(dma.channel);
	dma.channel = 0;
}
