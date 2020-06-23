#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"

#include "I2c.h"

static void i2cSlaveStateMachineReset(struct I2cSlaveStateMachine *machine);
static void i2cSlaveOnInitialEvent(struct I2cSlaveStateMachine *machine);
static void i2cSlaveOnAddressHighByteReceived(struct I2cSlaveStateMachine *machine);
static void i2cSlaveOnAddressLowByteReceived(struct I2cSlaveStateMachine *machine);
static const struct I2cBank *i2cGetBank(uint8_t number);
static void i2cSlaveOnDataByteReceived(struct I2cSlaveStateMachine *machine);
static uint16_t i2cNextRegisterAddress(uint16_t address);
static void i2cSlaveOnByteTransmit(struct I2cSlaveStateMachine *machine);

static const struct I2cBank i2cNopBank =
{
	.number = 0xff,
	.readByte = 0,
	.writeByte = 0
};

void i2cSlaveInitialise(struct I2cSlaveStateMachine *machine)
{
	if (!machine)
		return;

	machine->transaction.bank = &i2cNopBank;
	machine->transaction.registerAddress = 0;

	const struct I2cBank *bank = i2cFirstBank;
	while (bank < i2cLastBank)
	{
		if (bank->readByte == &i2cRamReadByte)
		{
			machine->transaction.bank = bank;
			break;
		}

		bank++;
	}

	i2cSlaveStateMachineReset(machine);
}

static void i2cSlaveStateMachineReset(struct I2cSlaveStateMachine *machine)
{
	i2cNackAndReleaseClockStretch();
	if (I2C2STAT & _I2C2STAT_RBF_MASK)
		(void) I2C2RCV;

	machine->state.onEvent = &i2cSlaveOnInitialEvent;
}

static void i2cSlaveOnInitialEvent(struct I2cSlaveStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_D_A_MASK | _I2C2STAT_S_MASK | _I2C2STAT_R_W_MASK | _I2C2STAT_RBF_MASK);
	if (maskedStat == (_I2C2STAT_S_MASK | _I2C2STAT_RBF_MASK))
	{
		i2cAckAndReleaseClockStretch();
		machine->transaction.deviceAddress = I2C2RCV;
		machine->state.onEvent = &i2cSlaveOnAddressHighByteReceived;
	}
	else if (maskedStat == (_I2C2STAT_S_MASK | _I2C2STAT_R_W_MASK | _I2C2STAT_RBF_MASK))
	{
		machine->transaction.deviceAddress = I2C2RCV;
		machine->transaction.registerStartAddress = machine->transaction.registerAddress;
		machine->transaction.registerCount = 0;
		machine->state.onEvent = &i2cSlaveOnByteTransmit;
		machine->state.onEvent(machine);
	}
	else
		i2cSlaveStateMachineReset(machine);
}

static void i2cSlaveOnAddressHighByteReceived(struct I2cSlaveStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_D_A_MASK | _I2C2STAT_S_MASK | _I2C2STAT_P_MASK | _I2C2STAT_R_W_MASK | _I2C2STAT_RBF_MASK);
	if (maskedStat != (_I2C2STAT_D_A_MASK | _I2C2STAT_S_MASK | _I2C2STAT_RBF_MASK))
	{
		i2cSlaveOnInitialEvent(machine);
		return;
	}

	i2cAckAndReleaseClockStretch();
	machine->transaction.registerAddressTemporaryHighByte = I2C2RCV;
	machine->state.onEvent = (I2cSlaveEventHandler) &i2cSlaveOnAddressLowByteReceived;
}

static void i2cSlaveOnAddressLowByteReceived(struct I2cSlaveStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_D_A_MASK | _I2C2STAT_S_MASK | _I2C2STAT_P_MASK | _I2C2STAT_R_W_MASK | _I2C2STAT_RBF_MASK);
	if (maskedStat != (_I2C2STAT_D_A_MASK | _I2C2STAT_S_MASK | _I2C2STAT_RBF_MASK))
	{
		i2cSlaveOnInitialEvent(machine);
		return;
	}

	i2cAckAndReleaseClockStretch();
	uint8_t bankNumber = (machine->transaction.registerAddressTemporaryHighByte >> 2) & 0x3f;
	machine->transaction.bank = i2cGetBank(bankNumber);
	machine->transaction.registerStartAddress = (((uint16_t) machine->transaction.registerAddressTemporaryHighByte << 8)) | I2C2RCV;
	machine->transaction.registerAddress = machine->transaction.registerStartAddress;
	machine->transaction.registerCount = 0;

	machine->state.onEvent = &i2cSlaveOnDataByteReceived;
}

static const struct I2cBank *i2cGetBank(uint8_t number)
{
	const struct I2cBank *bank = i2cFirstBank;
	while (bank < i2cLastBank)
	{
		if (bank->number == number)
			return bank;

		bank++;
	}

	return &i2cNopBank;
}

static void i2cSlaveOnDataByteReceived(struct I2cSlaveStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_D_A_MASK | _I2C2STAT_S_MASK | _I2C2STAT_P_MASK | _I2C2STAT_R_W_MASK | _I2C2STAT_RBF_MASK);
	if (maskedStat != (_I2C2STAT_D_A_MASK | _I2C2STAT_S_MASK | _I2C2STAT_RBF_MASK))
	{
		struct I2cEvent event =
		{
			.header = EVENT_QUEUE_HEADER_INIT(I2C_MODULE_ID, I2C_EVENT_SLAVE_TRANSACTION_DONE),
			.as =
			{
				.slaveTransactionDone =
				{
					.deviceAddress = (machine->transaction.deviceAddress >> 1) & 0x7f,
					.registerAddress = machine->transaction.registerStartAddress,
					.registerCount = machine->transaction.registerCount,
					.flags =
					{
						.bits =
						{
							.isWrite = 1
						}
					}
				}
			}
		};

		if (machine->configuration.isEventBroadcastEnabled)
			xQueueSendToBack(i2cEvents, &event, portMAX_DELAY);

		if (machine->transaction.bank->onTransactionDone)
			machine->transaction.bank->onTransactionDone();

		i2cSlaveOnInitialEvent(machine);
		return;
	}

	uint16_t addressInBank = machine->transaction.registerAddress & I2C_REGISTER_ADDRESS_OFFSET_MASK;
	if (machine->transaction.bank->writeByte && machine->transaction.bank->writeByte(addressInBank, I2C2RCV))
		i2cAckAndReleaseClockStretch();
	else
		i2cNackAndReleaseClockStretch();

	machine->transaction.registerAddress = i2cNextRegisterAddress(machine->transaction.registerAddress);
	if (machine->transaction.registerCount < 0xffffu)
		machine->transaction.registerCount++;
}

static uint16_t i2cNextRegisterAddress(uint16_t address)
{
	uint16_t overflow = (address & 0xfc) + 1024;
	uint16_t nextAddress = address + 1;
	if (nextAddress == overflow)
		return address & I2C_REGISTER_ADDRESS_BANK_MASK;

	return nextAddress;
}

static void i2cSlaveOnByteTransmit(struct I2cSlaveStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_S_MASK | _I2C2STAT_P_MASK | _I2C2STAT_R_W_MASK | _I2C2STAT_TBF_MASK | _I2C2STAT_ACKSTAT_MASK);
	if (maskedStat != (_I2C2STAT_S_MASK | _I2C2STAT_R_W_MASK))
	{
		struct I2cEvent event =
		{
			.header = EVENT_QUEUE_HEADER_INIT(I2C_MODULE_ID, I2C_EVENT_SLAVE_TRANSACTION_DONE),
			.as =
			{
				.slaveTransactionDone =
				{
					.deviceAddress = (machine->transaction.deviceAddress >> 1) & 0x7f,
					.registerAddress = machine->transaction.registerStartAddress,
					.registerCount = machine->transaction.registerCount,
					.flags =
					{
						.bits =
						{
							.isWrite = 0
						}
					}
				}
			}
		};

		if (machine->configuration.isEventBroadcastEnabled)
			xQueueSendToBack(i2cEvents, &event, portMAX_DELAY);

		if (machine->transaction.bank->onTransactionDone)
			machine->transaction.bank->onTransactionDone();

		i2cSlaveOnInitialEvent(machine);
		return;
	}

	uint16_t addressInBank = machine->transaction.registerAddress & I2C_REGISTER_ADDRESS_OFFSET_MASK;
	if (machine->transaction.bank->readByte)
		I2C2TRN = machine->transaction.bank->readByte(addressInBank);
	else
		I2C2TRN = 0x00;

	i2cReleaseClockStretchAfterTrn();
	machine->transaction.registerAddress = i2cNextRegisterAddress(machine->transaction.registerAddress);
	if (machine->transaction.registerCount < 0xffffu)
		machine->transaction.registerCount++;
}
