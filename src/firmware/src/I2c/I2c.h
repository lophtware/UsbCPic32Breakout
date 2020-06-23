#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_I2C_I2C_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_I2C_I2C_H
#include <stdint.h>
#include <stdbool.h>
#include "../FreeRtos.h"
#include "../I2c.h"

#define I2C_INITIALISED 0
#define I2C_REINITIALISE 1
#define I2C_REINITIALISE_AND_ENABLE 2

#define I2C_STATE_MACHINE_IDLE 0x00
#define I2C_STATE_MACHINE_SLAVE 0x01
#define I2C_STATE_MACHINE_MASTER 0x02
#define I2C_STATE_MACHINE_MASTER_NEW_TRANSACTION 0x04
#define I2C_STATE_MACHINE_REINITIALISE 0x8000

#define I2C_REGISTER_ADDRESS_BANK_MASK 0xfc00
#define I2C_REGISTER_ADDRESS_OFFSET_MASK 0x03ff

struct I2cMasterTransactionState
{
	struct I2cMasterTransaction transaction;
	const uint8_t *outPtr;
	uint8_t *inPtr;
	struct I2cMasterOnChunkReceivedEventArgs inChunkArgs;
	bool canRecoverFromCollision;
	uint8_t deviceAddress;
};

struct I2cSlaveTransactionState
{
	uint8_t deviceAddress;
	uint8_t registerAddressTemporaryHighByte;
	uint16_t registerStartAddress;
	uint16_t registerAddress;
	uint16_t registerCount;
	const struct I2cBank *bank;
};

struct I2cMasterStateMachine;
typedef void (*I2cMasterEventHandler)(struct I2cMasterStateMachine *machine);

struct I2cSlaveStateMachine;
typedef void (*I2cSlaveEventHandler)(struct I2cSlaveStateMachine *machine);

struct I2cMasterStateTransition
{
	I2cMasterEventHandler onEvent;
	I2cMasterEventHandler onTimeout;
	TickType_t timeout;
};

struct I2cSlaveStateTransition
{
	I2cSlaveEventHandler onEvent;
};

struct I2cMasterStateMachine
{
	struct
	{
		struct I2cMasterTimeouts timeouts;
	} configuration;

	struct I2cMasterStateTransition state;
	struct I2cMasterTransactionState transaction;
};

struct I2cSlaveStateMachine
{
	struct
	{
		bool isEventBroadcastEnabled;
	} configuration;

	struct I2cSlaveStateTransition state;
	struct I2cSlaveTransactionState transaction;
};

struct I2cState
{
	struct I2cMasterStateMachine master;
	struct I2cSlaveStateMachine slave;

	uint8_t forceReinitialisation;
};

__attribute__((packed))
union I2cNvmPage
{
	uint8_t page[2048];
	struct
	{
		uint8_t ramInitialisation[I2C_RAM_SIZE_BYTES];

		__attribute__((aligned(8)))
		union
		{
			uint8_t asBytes[I2C_ROM_SIZE_BYTES];
			uint64_t asDwords[I2C_ROM_SIZE_BYTES / 8];
		} rom;
	} data;
};

__attribute__((section(".i2c_nvm"), aligned(2048)))
extern volatile const union I2cNvmPage i2cNvmPage;

extern TaskHandle_t i2cTaskHandle;
extern QueueHandle_t i2cEvents;
extern QueueHandle_t i2cMasterTransactions;
extern struct I2cMasterTimeouts i2cMasterTimeouts;
const struct I2cBank *i2cFirstBank, *i2cLastBank;

extern void i2cTask(void *args);

extern void i2cRomInitialise(void);
extern bool i2cNvmStoreAndReset(const uint8_t *contents, uint16_t offset, uint16_t count);

extern void i2cRamInitialise(void);

extern void i2cSlaveInitialise(struct I2cSlaveStateMachine *machine);

extern void i2cMasterInitialise(struct I2cMasterStateMachine *machine, const struct I2cMasterTimeouts *timeouts);
extern void i2cMasterOnInitialEvent(struct I2cMasterStateMachine *machine);

static inline void i2cDelayMinimumSetupTime(void)
{
#if configPERIPHERAL_CLOCK_HZ != 24000000
#error The I2C 250ns minimum data setup time will be wrong if the peripheral clock is not 24MHz
#endif
	_nop(); _nop();
	_nop(); _nop();
	_nop(); _nop();
	_nop();
}

static inline void i2cDelayMaximumRiseTime(void)
{
#if configPERIPHERAL_CLOCK_HZ != 24000000
#error The I2C 1us maximum rise time will be wrong if the peripheral clock is not 24MHz
#endif
	_nop(); _nop(); _nop(); _nop();
	_nop(); _nop(); _nop(); _nop();
	_nop(); _nop(); _nop(); _nop();
	_nop(); _nop(); _nop(); _nop();
	_nop(); _nop(); _nop(); _nop();
	_nop(); _nop(); _nop(); _nop();
	_nop();
}

static inline void i2cAckAndReleaseClockStretch(void)
{
	/* THERE IS NO MANUAL ACK / NACK CONTROL IN THE PIC I2C PERIPHERAL !!! :( */
	i2cDelayMinimumSetupTime();
	I2C2CONSET = _I2C2CON_SCLREL_MASK;
}

static inline void i2cNackAndReleaseClockStretch(void)
{
	/* THERE IS NO MANUAL ACK / NACK CONTROL IN THE PIC I2C PERIPHERAL !!! :( */
	i2cDelayMinimumSetupTime();
	i2cDelayMaximumRiseTime();
	I2C2CONSET = _I2C2CON_SCLREL_MASK;
}

static inline void i2cReleaseClockStretchAfterTrn(void)
{
	i2cDelayMinimumSetupTime();
	I2C2CONSET = _I2C2CON_SCLREL_MASK;
}

static inline uint32_t getHeapSectionSize(void)
{
	return __builtin_section_size(".heap");
}

#define ALIGNED(x, y) ((((uint32_t) (x)) + ((y) - 1)) & ~((y) - 1))
static inline void *getHeapSection(void)
{
	return (void *) ALIGNED(__builtin_section_begin(".heap"), 8);
}

#endif
