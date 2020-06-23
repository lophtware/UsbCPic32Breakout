#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_I2C_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_I2C_H
#include <stdint.h>
#include <stdbool.h>
#include "FreeRtos.h"
#include "EventQueueHeader.h"

#define I2C_MODULE_ID 0x04
#define I2C_EVENT_SLAVE_TRANSACTION_DONE 0x02

#define I2C_DEFAULT_ADDRESS 0x31
#define I2C_BRG_MINIMUM_COUNT 11
#define I2C_BRG_NO_MORE_THAN_100KHZ 121

#define I2C_HOLD_TIME_100NS 0
#define I2C_HOLD_TIME_300NS 1

#define I2C_ROM_SIZE_BYTES 1024
#define I2C_RAM_SIZE_BYTES 1024

#define I2C_BANK_BYTE_Y(x, y) ((uint8_t) (((x) >> (y)) & 0xff))
#define I2C_BANK_BYTE_0(x) I2C_BANK_BYTE_Y(x, 0)
#define I2C_BANK_BYTE_1(x) I2C_BANK_BYTE_Y(x, 8)
#define I2C_BANK_BYTE_2(x) I2C_BANK_BYTE_Y(x, 16)
#define I2C_BANK_BYTE_3(x) I2C_BANK_BYTE_Y(x, 24)

#define I2C_BANK_WORD_REGISTER(baseAddress, reg) \
	case (baseAddress) + 0: return I2C_BANK_BYTE_0(reg); \
	case (baseAddress) + 1: return I2C_BANK_BYTE_1(reg); \
	case (baseAddress) + 2: return I2C_BANK_BYTE_2(reg); \
	case (baseAddress) + 3: return I2C_BANK_BYTE_3(reg);

#define I2C_BANK_HALF_WORD_REGISTER(baseAddress, reg) \
	case (baseAddress) + 0: return I2C_BANK_BYTE_0(reg); \
	case (baseAddress) + 1: return I2C_BANK_BYTE_1(reg);

#define I2C_BANK_BYTE_REGISTER(baseAddress, reg) \
	case (baseAddress) + 0: return I2C_BANK_BYTE_0(reg);

struct I2cMasterTimeouts
{
	TickType_t waitingForAddressAck;
	TickType_t waitingForStopBit;
	TickType_t waitingForSlaveDataAck;
	TickType_t waitingForSlaveDataIn;
	TickType_t waitingForMasterAck;
};

struct I2cConfiguration
{
	__attribute__((packed))
	union
	{
		uint32_t raw;

		struct
		{
			unsigned int : 8;
			unsigned int isSmbusLevels : 1;
			unsigned int isSlewRateControlDisabled : 1;
			unsigned int : 1;
			unsigned int isStrictAddressMode : 1;
			unsigned int : 3;
			unsigned int isI2cEnabled : 1;
			unsigned int : 3;
			unsigned int isHoldTime300ns : 1;
			unsigned int : 12;
		} bits;
	} con;

	uint16_t brg;
	uint8_t add;
	uint8_t msk;

	struct
	{
		struct I2cMasterTimeouts timeouts;
	} master;

	struct
	{
		struct
		{
			uint16_t protectedAddressMask;
			union
			{
				uint8_t all;

				struct
				{
					unsigned int isWriteProtected : 1;
					unsigned int : 7;
				} bits;
			} flags;
		} rom;

		struct
		{
			uint16_t protectedAddressMask;
			union
			{
				uint8_t all;

				struct
				{
					unsigned int isWriteProtected : 1;
					unsigned int : 7;
				} bits;
			} flags;
		} ram;

		bool isEventBroadcastEnabled;
	} slave;
};

struct I2cBank
{
	uint8_t number;
	uint8_t (*readByte)(uint16_t address);
	bool (*writeByte)(uint16_t address, uint8_t value);
	void (*onTransactionDone)(void);
};

struct I2cMasterOnChunkReceivedEventArgs
{
	uint8_t chunkNumber;
	uint8_t length;
	bool nack;
	bool retry;
};

struct I2cMasterTransaction;
typedef bool (*I2cMasterTransactionOnDone)(const struct I2cMasterTransaction *transaction);
typedef void (*I2cMasterTransactionOnChunkReceived)(const struct I2cMasterTransaction *transaction, struct I2cMasterOnChunkReceivedEventArgs *args);

struct I2cMasterTransaction
{
	const uint8_t *out;
	union
	{
		uint8_t *asDirect;
		uint8_t *(*asDelegate)(void);
	} in;

	uint16_t outLength;
	uint16_t inLength;

	I2cMasterTransactionOnDone onDone;
	I2cMasterTransactionOnChunkReceived onChunkReceived;
	void *context;

	union
	{
		struct
		{
			unsigned int writeDone : 1;
			unsigned int success : 1;
			unsigned int collision : 1;
			unsigned int timeout : 1;
			unsigned int addressNack : 1;
			unsigned int dataNack : 1;
			unsigned int : 2;
		} bits;

		uint8_t all;
	} statusFlags;

	union
	{
		struct
		{
			unsigned int isInBufferDelegate : 1;
			unsigned int : 7;
		} bits;

		uint8_t all;
	} flags;

	uint8_t deviceAddress;
	uint8_t inChunkLength;
};

struct I2cSlaveAddress
{
	union
	{
		uint32_t raw;
		struct
		{
			uint8_t address;
			uint8_t mask;
		} fields;
	} as;
};

struct I2cEvent
{
	struct EventQueueHeader header;

	union
	{
		uint8_t raw[6];

		struct
		{
			uint16_t registerAddress;
			uint16_t registerCount;
			uint8_t deviceAddress;
			union
			{
				struct
				{
					unsigned int isWrite : 1;
					unsigned int : 7;
				} bits;
				uint8_t all;
			} flags;
		} slaveTransactionDone;
	} as;
};

extern void i2cInitialise(const struct I2cBank *banks, uint8_t numberOfBanks, QueueHandle_t eventQueue);
extern void i2cApplyConfiguration(const struct I2cConfiguration *i2c);
extern void i2cEnable(void);
extern void i2cDisable(void);
extern void i2cEnableSmbusLevels(void);
extern void i2cDisableSmbusLevels(void);
extern bool i2cAreSmbusLevelsEnabled(void);
extern void i2cEnableSlewRateControl(void);
extern void i2cDisableSlewRateControl(void);
extern bool i2cIsSlewRateControlEnabled(void);
extern void i2cSetHoldTime(uint8_t isHoldTime300ns);
extern bool i2cIsHoldTime(uint8_t isHoldTime300ns);

extern void i2cRamSetProtectedAddressMask(uint16_t mask);
extern uint16_t i2cRamGetProtectedAddressMask(void);
extern void i2cRamEnableWriteProtection(void);
extern void i2cRamDisableWriteProtection(void);
extern bool i2cRamIsWriteProtected(void);
extern uint8_t i2cRamReadByte(uint16_t address);
extern bool i2cRamWriteByte(uint16_t address, uint8_t value);
extern void i2cRamReadBytesUnprotected(uint8_t *buffer, uint16_t address, uint16_t count);
extern bool i2cRamWriteBytesUnprotected(uint16_t address, const uint8_t *contents, uint16_t count);
extern bool i2cRamInitialisationStoreAndReset(const uint8_t *contents, uint16_t offset, uint16_t count);
extern volatile const uint8_t *i2cRamInitialisationGetContents(void);

extern void i2cRomSetProtectedAddressMask(uint16_t mask);
extern uint16_t i2cRomGetProtectedAddressMask(void);
extern void i2cRomEnableWriteProtection(void);
extern void i2cRomDisableWriteProtection(void);
extern bool i2cRomIsWriteProtected(void);
extern uint8_t i2cRomReadByte(uint16_t address);
extern bool i2cRomWriteByte(uint16_t address, uint8_t value);
extern void i2cRomReadBytesUnprotected(uint8_t *buffer, uint16_t address, uint16_t count);
extern bool i2cRomWriteBytesUnprotected(uint16_t address, const uint8_t *contents, uint16_t count);
extern void i2cRomOnTransactionDone(void);
extern bool i2cRomStoreAndReset(const uint8_t *contents, uint16_t offset, uint16_t count);
extern volatile const uint8_t *i2cRomGetContents(void);

extern bool i2cMasterSend(const struct I2cMasterTransaction *transaction);
extern bool i2cMasterIsIdleFromIsr(void);
extern uint16_t i2cMasterGetBaudRate(void);
extern void i2cMasterSetBaudRate(uint16_t counter);
extern void i2cMasterGetTimeouts(struct I2cMasterTimeouts *timeouts);
extern void i2cMasterSetTimeouts(const struct I2cMasterTimeouts *timeouts);

extern void i2cSlaveEnableEventBroadcast(void);
extern void i2cSlaveDisableEventBroadcast(void);
extern bool i2cSlaveIsEventBroadcastEnabled(void);
extern void i2cSlaveSetAddress(const struct I2cSlaveAddress address);
extern struct I2cSlaveAddress i2cSlaveGetAddress(void);

#endif
