#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_SPI_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_SPI_H
#include <stdint.h>
#include <stdbool.h>
#include "Pins.h"

#define SPI_BRG_MINIMUM_COUNT 0
#define SPI_BRG_MAXIMUM_COUNT ((1 << 13) - 1)
#define SPI_BRG_TYPICAL_1MHZ 11

struct SpiSlaveSelectPin
{
	struct Pin handle;
	bool isActiveHigh;
};

struct SpiSlaveConfiguration
{
	struct
	{
		uint32_t con;
		uint16_t con2;
		uint16_t brg;
	} registers;

	struct
	{
		struct SpiSlaveSelectPin pin;
		uint8_t delayMicroseconds;
	} slaveSelect;
};

struct SpiConfiguration
{
	struct SpiSlaveConfiguration slaves[8];
};

struct SpiMasterOnChunkReceivedEventArgs
{
	uint16_t length;
	uint16_t consumed;
	uint16_t chunkNumber;
	uint8_t pollIntervalTicks;
	TaskHandle_t pollTaskHandle;
};

struct SpiMasterTransaction;
typedef bool (*SpiMasterTransactionOnDone)(const struct SpiMasterTransaction *transaction);
typedef void (*SpiMasterTransactionOnChunkReceived)(const struct SpiMasterTransaction *transaction, struct SpiMasterOnChunkReceivedEventArgs *args);
typedef bool (*SpiMasterTransactionOnRepeat)(struct SpiMasterTransaction *transaction);

struct SpiMasterTransaction
{
	const uint8_t *out;
	uint8_t *in;
	uint16_t outLength;
	uint16_t inLength;

	SpiMasterTransactionOnDone onDone;
	SpiMasterTransactionOnChunkReceived onChunkReceived;
	SpiMasterTransactionOnRepeat onRepeat;
	void *context;

	union
	{
		struct
		{
			unsigned int timeout : 1;
			unsigned int done : 1;
			unsigned int : 6;
		} bits;

		uint8_t all;
	} statusFlags;

	union
	{
		struct
		{
			unsigned int slaveAddress : 3;
			unsigned int : 5;
		} bits;

		uint8_t all;
	} flags;

	uint16_t inChunkLength;
};

extern void spiInitialise(void);
extern void spiApplyConfiguration(const struct SpiConfiguration *spi);

extern bool spiMasterSend(const struct SpiMasterTransaction *transaction);
extern bool spiMasterIsIdleFromIsr(void);

extern void spiSlavesAssignSelectPin(uint8_t index, const struct SpiSlaveSelectPin pin);
extern void spiSlavesUnassignSelectPin(const struct Pin pin);
extern void spiSlavesGetConfiguration(uint8_t index, struct SpiSlaveConfiguration *configuration);
extern void spiSlavesSetConfiguration(uint8_t index, const struct SpiSlaveConfiguration *configuration);

#endif
