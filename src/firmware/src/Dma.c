#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/kmem.h>

#include "FreeRtos.h"

#include "Dma.h"

#define DMA_CHANNEL_START_TRANSACTION(channel, transaction) \
	{ \
		DCH##channel##INT = \
			((transaction)->isrOnDone ? _DCH0INT_CHBCIE_MASK : 0) | \
			((transaction)->isrOnError ? _DCH0INT_CHERIE_MASK : 0) | \
			((transaction)->isrOnAbort ? _DCH0INT_CHTAIE_MASK : 0); \
	\
		DCH##channel##ECON = \
			((transaction)->abortIrq << _DCH0ECON_CHAIRQ_POSITION) | \
			((transaction)->transferIrq << _DCH0ECON_CHSIRQ_POSITION) | \
			((transaction)->transferIrq != DMA_IRQ_UNUSED ? _DCH0ECON_SIRQEN_MASK : 0) | \
			((transaction)->abortIrq != DMA_IRQ_UNUSED ? _DCH0ECON_AIRQEN_MASK : 0); \
	\
		DCH##channel##SSA = KVA_TO_PA((transaction)->source.ptr); \
		DCH##channel##SSIZ = (transaction)->source.length; \
		DCH##channel##DSA = KVA_TO_PA((transaction)->destination.ptr); \
		DCH##channel##DSIZ = (transaction)->destination.length; \
	\
		DCH##channel##CSIZ = (transaction)->cellSize; \
	\
		DCH##channel##CON = \
			_DCH0CON_CHEN_MASK | \
			((transaction)->isContinuous ? _DCH0CON_CHAEN_MASK : 0) | \
			(((transaction)->priority & 0x03) << _DCH0CON_CHPRI_POSITION); \
	\
		if ((transaction)->needsStarting) \
			DCH##channel##ECONSET = _DCH0ECON_CFORCE_MASK; \
	}

#define ALL_DCHINT_IE 0xffff0000ul

static struct DmaTransaction dma0Transaction;
static struct DmaTransaction dma1Transaction;
static struct DmaTransaction dma2Transaction;
static struct DmaTransaction dma3Transaction;

static bool dmaIsChannelAllocated[4];

void dmaInitialise(void)
{
	DCRCCON = 0;
	DMACON = _DMACON_ON_MASK;
	DCH0CON = 0;
	DCH1CON = 0;
	DCH2CON = 0;
	DCH3CON = 0;

	const int priority = 7;
	IPC24bits.DMA0IP = priority;
	IPC24bits.DMA0IS = 0;
	IPC24bits.DMA1IP = priority;
	IPC24bits.DMA1IS = 0;
	IPC25bits.DMA2IP = priority;
	IPC25bits.DMA2IS = 0;
	IPC25bits.DMA3IP = priority;
	IPC25bits.DMA3IS = 0;
}

void dmaSuspend(void)
{
	DMACONSET = _DMACON_SUSPEND_MASK;
	while (DCH0CONbits.CHBUSY || DCH1CONbits.CHBUSY || DCH2CONbits.CHBUSY || DCH3CONbits.CHBUSY)
		;;
}

void dmaResume(void)
{
	DMACONCLR = _DMACON_SUSPEND_MASK;
}

void dmaTransactionStart(const struct DmaTransaction *transaction)
{
	if (!transaction)
		return;

	switch (transaction->channel)
	{
		case 1:
			DMA_CHANNEL_START_TRANSACTION(0, transaction);
			memcpy(&dma0Transaction, transaction, sizeof(struct DmaTransaction));
			break;

		case 2:
			DMA_CHANNEL_START_TRANSACTION(1, transaction);
			memcpy(&dma1Transaction, transaction, sizeof(struct DmaTransaction));
			break;

		case 3:
			DMA_CHANNEL_START_TRANSACTION(2, transaction);
			memcpy(&dma2Transaction, transaction, sizeof(struct DmaTransaction));
			break;

		case 4:
			DMA_CHANNEL_START_TRANSACTION(3, transaction);
			memcpy(&dma3Transaction, transaction, sizeof(struct DmaTransaction));
			break;

		default:
			break;
	}
}

void dmaTransactionAbort(const struct DmaTransaction *transaction)
{
	if (!transaction)
		return;

	switch (transaction->channel)
	{
		case 1:
			DCH0ECONSET = _DCH0ECON_CABORT_MASK;
			while (DCH0CONbits.CHBUSY)
				;;
			break;

		case 2:
			DCH1ECONSET = _DCH1ECON_CABORT_MASK;
			while (DCH1CONbits.CHBUSY)
				;;
			break;

		case 3:
			DCH2ECONSET = _DCH2ECON_CABORT_MASK;
			while (DCH2CONbits.CHBUSY)
				;;
			break;

		case 4:
			DCH3ECONSET = _DCH3ECON_CABORT_MASK;
			while (DCH3CONbits.CHBUSY)
				;;
			break;

		default:
			break;
	}
}

DmaChannel dmaChannelTryAllocate(void)
{
	DmaChannel channel = 0;
	vTaskSuspendAll();
	for (uint8_t i = 0; i < sizeof(dmaIsChannelAllocated) / sizeof(bool); i++)
	{
		if (!dmaIsChannelAllocated[i])
		{
			dmaIsChannelAllocated[i] = true;
			channel = i;
			break;
		}
	}
	xTaskResumeAll();
	return channel;
}

void dmaChannelFree(DmaChannel channel)
{
	if (channel >= sizeof(dmaIsChannelAllocated) / sizeof(bool))
		return;

	dmaIsChannelAllocated[channel] = false;
}

bool dmaChannelIsBusy(DmaChannel channel)
{
	switch (channel)
	{
		case 1:
			return DCH0CONbits.CHBUSY ? true : false;

		case 2:
			return DCH1CONbits.CHBUSY ? true : false;

		case 3:
			return DCH2CONbits.CHBUSY ? true : false;

		case 4:
			return DCH3CONbits.CHBUSY ? true : false;

		default:
			return false;
	}
}

uint16_t dmaChannelBytesTransferred(DmaChannel channel)
{
	uint16_t bytes = 0;
	switch (channel)
	{
		case 1:
			bytes = DCH0DPTR;
			if (!DCH0CONbits.CHBUSY)
				bytes = DCH0DSIZ;
			break;

		case 2:
			bytes = DCH1DPTR;
			if (!DCH1CONbits.CHBUSY)
				bytes = DCH1DSIZ;
			break;

		case 3:
			bytes = DCH2DPTR;
			if (!DCH2CONbits.CHBUSY)
				bytes = DCH2DSIZ;
			break;

		case 4:
			bytes = DCH3DPTR;
			if (!DCH3CONbits.CHBUSY)
				bytes = DCH3DSIZ;
			break;

		default:
			bytes = 0;
			break;
	}

	return bytes;
}

#define DMA_ISR_IMPLEMENTATION(channel) \
	{ \
		IFS3CLR = _IFS3_DMA##channel##IF_MASK; \
		DCH##channel##INTCLR = ALL_DCHINT_IE; \
	\
		bool reprimeChannel = false; \
		if (DCH##channel##INTbits.CHBCIF && dma##channel##Transaction.isrOnDone) \
			reprimeChannel = dma##channel##Transaction.isrOnDone(channel + 1, &dma##channel##Transaction); \
		else if (DCH##channel##INTbits.CHTAIF && dma##channel##Transaction.isrOnAbort) \
			reprimeChannel = dma##channel##Transaction.isrOnAbort(channel + 1, &dma##channel##Transaction); \
		else if (DCH##channel##INTbits.CHERIF && dma##channel##Transaction.isrOnError) \
			reprimeChannel = dma##channel##Transaction.isrOnError(channel + 1, &dma##channel##Transaction); \
	\
		if (reprimeChannel) \
		{ \
			DMA_CHANNEL_START_TRANSACTION(channel, &dma##channel##Transaction); \
		} \
	}

static void __attribute__((interrupt(), vector(_DMA0_VECTOR), nomips16, used)) _dma0Interrupt(void)
{
	DMA_ISR_IMPLEMENTATION(0);
}

static void __attribute__((interrupt(), vector(_DMA1_VECTOR), nomips16, used)) _dma1Interrupt(void)
{
	DMA_ISR_IMPLEMENTATION(1);
}

static void __attribute__((interrupt(), vector(_DMA2_VECTOR), nomips16, used)) _dma2Interrupt(void)
{
	DMA_ISR_IMPLEMENTATION(2);
}

static void __attribute__((interrupt(), vector(_DMA3_VECTOR), nomips16, used)) _dma3Interrupt(void)
{
	DMA_ISR_IMPLEMENTATION(3);
}
