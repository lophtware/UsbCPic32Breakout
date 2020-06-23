#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_DMA_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_DMA_H
#include <stdint.h>
#include <stdbool.h>

#define DMA_CHANNEL_PRIORITY_LOWEST 0
#define DMA_CHANNEL_PRIORITY_LOW 1
#define DMA_CHANNEL_PRIORITY_HIGH 2
#define DMA_CHANNEL_PRIORITY_HIGHEST 3

#define DMA_IRQ_UNUSED 0

typedef uint8_t DmaChannel;

struct DmaSourcePointer
{
	const void *ptr;
	uint16_t length;
};

struct DmaDestinationPointer
{
	void *ptr;
	uint16_t length;
};

struct DmaTransaction;
typedef bool (*DmaIsrCallback)(uint8_t channel, struct DmaTransaction *transaction);

struct DmaTransaction
{
	struct DmaSourcePointer source;
	struct DmaDestinationPointer destination;
	uint16_t cellSize;
	uint8_t priority;
	uint8_t transferIrq;
	uint8_t abortIrq;
	bool isContinuous;
	bool needsStarting;
	DmaChannel channel;
	DmaIsrCallback isrOnDone;
	DmaIsrCallback isrOnError;
	DmaIsrCallback isrOnAbort;
};

extern void dmaInitialise(void);
extern void dmaSuspend(void);
extern void dmaResume(void);
extern void dmaTransactionStart(const struct DmaTransaction *transaction);
extern void dmaTransactionAbort(const struct DmaTransaction *transaction);
extern DmaChannel dmaChannelTryAllocate(void);
extern void dmaChannelFree(DmaChannel channel);
extern bool dmaChannelIsBusy(DmaChannel channel);
extern uint16_t dmaChannelBytesTransferred(DmaChannel channel);

#endif
