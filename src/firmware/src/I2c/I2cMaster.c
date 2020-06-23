#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../FreeRtos.h"

#include "I2c.h"

#define WAITING_FOR_START_BIT_SENT_TIMEOUT pdMS_TO_TICKS(100)
#define WAITING_FOR_STOP_BIT_SENT_TIMEOUT pdMS_TO_TICKS(100)

static void i2cMasterStateMachineReset(struct I2cMasterStateMachine *machine);
static void i2cMasterOnStartBitSent(struct I2cMasterStateMachine *machine);
static void i2cMasterOnBusCollision(struct I2cMasterStateMachine *machine);
static void i2cMasterOnBusCollisionWaitingForStopBit(struct I2cMasterStateMachine *machine);
static void i2cMasterOnAddressSent(struct I2cMasterStateMachine *machine);
static void i2cMasterOnStopBitSent(struct I2cMasterStateMachine *machine);
static void i2cMasterDispatchDone(struct I2cMasterStateMachine *machine);
static void i2cMasterOnByteSent(struct I2cMasterStateMachine *machine);
static void i2cMasterOnByteReceived(struct I2cMasterStateMachine *machine);
static void i2cMasterDispatchChunkReceived(struct I2cMasterStateMachine *machine);
static void i2cMasterOnAckd(struct I2cMasterStateMachine *machine);
static void i2cMasterOnTransactionTimeout(struct I2cMasterStateMachine *machine);

bool i2cMasterSend(const struct I2cMasterTransaction *transaction)
{
	if (!transaction ||
		!transaction->onDone ||
		(transaction->inChunkLength > 0 && transaction->inLength > 0 && !transaction->onChunkReceived) ||
		(transaction->outLength > 0 && !transaction->out) ||
		(transaction->inLength > 0 && ((transaction->flags.bits.isInBufferDelegate && !transaction->in.asDelegate) || !transaction->in.asDirect)) ||
		(transaction->inLength == 0 && transaction->outLength == 0))
	{
		return false;
	}

	if (xQueueSendToBack(i2cMasterTransactions, transaction, pdMS_TO_TICKS(100)) == pdPASS)
	{
		xTaskNotify(i2cTaskHandle, I2C_STATE_MACHINE_MASTER_NEW_TRANSACTION, eSetBits);
		return true;
	}

	return false;
}

void i2cMasterInitialise(struct I2cMasterStateMachine *machine, const struct I2cMasterTimeouts *timeouts)
{
	memcpy(&machine->configuration.timeouts, timeouts, sizeof(struct I2cMasterTimeouts));
	i2cMasterStateMachineReset(machine);
}

static void i2cMasterStateMachineReset(struct I2cMasterStateMachine *machine)
{
	machine->state.onEvent = &i2cMasterOnInitialEvent;
	machine->state.onTimeout = (I2cMasterEventHandler) 0;
	machine->state.timeout = portMAX_DELAY;
}

void i2cMasterOnInitialEvent(struct I2cMasterStateMachine *machine)
{
	if (xQueueReceive(i2cMasterTransactions, &machine->transaction.transaction, 0) == pdPASS)
	{
		machine->transaction.canRecoverFromCollision = true;
		machine->transaction.transaction.statusFlags.all = 0;

		if ((I2C2STAT & _I2C2STAT_S_MASK) == 0)
		{
			I2C2STATCLR = _I2C2STAT_BCL_MASK;
			I2C2CONSET = _I2C2CON_SEN_MASK;
			machine->state.onEvent = &i2cMasterOnStartBitSent;
			machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
			machine->state.timeout = WAITING_FOR_START_BIT_SENT_TIMEOUT;
		}
		else
			i2cMasterOnBusCollision(machine);
	}
	else
		i2cMasterStateMachineReset(machine);
}

static void i2cMasterOnStartBitSent(struct I2cMasterStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_BCL_MASK | _I2C2STAT_TBF_MASK);
	if (maskedStat & _I2C2STAT_BCL_MASK)
	{
		i2cMasterOnBusCollision(machine);
		return;
	}

	uint8_t isRead = (machine->transaction.transaction.outLength == 0) ? 1 : 0;
	I2C2TRN = (machine->transaction.transaction.deviceAddress << 1) | isRead;

	machine->transaction.canRecoverFromCollision = true;
	machine->transaction.transaction.statusFlags.all = 0;
	machine->transaction.transaction.statusFlags.bits.writeDone = isRead ? 1 : 0;
	machine->transaction.inChunkArgs.chunkNumber = 0;
	machine->transaction.inChunkArgs.length = 0;

	machine->transaction.outPtr = machine->transaction.transaction.out;
	machine->transaction.inPtr = machine->transaction.transaction.flags.bits.isInBufferDelegate
		? machine->transaction.transaction.in.asDelegate()
		: machine->transaction.transaction.in.asDirect;

	machine->state.onEvent = &i2cMasterOnAddressSent;
	machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
	machine->state.timeout = machine->configuration.timeouts.waitingForAddressAck;
}

static void i2cMasterOnBusCollision(struct I2cMasterStateMachine *machine)
{
	I2C2STATCLR = _I2C2STAT_BCL_MASK;
	machine->transaction.transaction.statusFlags.bits.collision = 1;
	machine->state.onEvent = &i2cMasterOnBusCollisionWaitingForStopBit;
	machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
	machine->state.timeout = machine->configuration.timeouts.waitingForStopBit;
}

static void i2cMasterOnBusCollisionWaitingForStopBit(struct I2cMasterStateMachine *machine)
{
	if (I2C2STAT & _I2C2STAT_S_MASK)
		return;

	if (machine->transaction.canRecoverFromCollision)
	{
		I2C2STATCLR = _I2C2STAT_BCL_MASK;
		I2C2CONSET = _I2C2CON_SEN_MASK;
		machine->state.onEvent = &i2cMasterOnStartBitSent;
		machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
		machine->state.timeout = WAITING_FOR_START_BIT_SENT_TIMEOUT;
	}
	else
		i2cMasterOnStopBitSent(machine);
}

static void i2cMasterOnAddressSent(struct I2cMasterStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_BCL_MASK | _I2C2STAT_ACKSTAT_MASK);
	if (maskedStat & _I2C2STAT_BCL_MASK)
	{
		i2cMasterOnBusCollision(machine);
		return;
	}

	if (maskedStat == _I2C2STAT_ACKSTAT_MASK)
	{
		I2C2CONSET = _I2C2CON_PEN_MASK;

		machine->transaction.transaction.statusFlags.bits.addressNack = 1;

		machine->state.onEvent = &i2cMasterOnStopBitSent;
		machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
		machine->state.timeout = WAITING_FOR_STOP_BIT_SENT_TIMEOUT;
		return;
	}

	if (machine->transaction.transaction.outLength > 0)
	{
		I2C2TRN = *machine->transaction.outPtr;
		machine->state.onEvent = &i2cMasterOnByteSent;
		machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
		machine->state.timeout = machine->configuration.timeouts.waitingForSlaveDataAck;
	}
	else
	{
		I2C2CONSET = _I2C2CON_RCEN_MASK;
		machine->state.onEvent = &i2cMasterOnByteReceived;
		machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
		machine->state.timeout = machine->configuration.timeouts.waitingForSlaveDataIn;
	}
}

static void i2cMasterOnStopBitSent(struct I2cMasterStateMachine *machine)
{
	machine->transaction.transaction.statusFlags.bits.success = (
		machine->transaction.transaction.statusFlags.all == 0 &&
		machine->transaction.transaction.outLength == 0 &&
		machine->transaction.transaction.inLength == 0)
		? 1
		: 0;

	i2cMasterDispatchDone(machine);
	i2cMasterStateMachineReset(machine);
}

static void i2cMasterDispatchDone(struct I2cMasterStateMachine *machine)
{
	while (!machine->transaction.transaction.onDone(&machine->transaction.transaction))
	{
		vTaskDelay(1);
	}
}

static void i2cMasterOnByteSent(struct I2cMasterStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & (_I2C2STAT_BCL_MASK | _I2C2STAT_ACKSTAT_MASK);
	if (maskedStat & _I2C2STAT_BCL_MASK)
	{
		i2cMasterOnBusCollision(machine);
		return;
	}
	else
		machine->transaction.canRecoverFromCollision = false;

	if (maskedStat == _I2C2STAT_ACKSTAT_MASK)
	{
		I2C2CONSET = _I2C2CON_PEN_MASK;

		machine->transaction.transaction.statusFlags.bits.dataNack = 1;

		machine->state.onEvent = &i2cMasterOnStopBitSent;
		machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
		machine->state.timeout = WAITING_FOR_STOP_BIT_SENT_TIMEOUT;
		return;
	}

	machine->transaction.outPtr++;
	if (--machine->transaction.transaction.outLength == 0)
	{
		machine->transaction.transaction.statusFlags.bits.writeDone = 1;
		if (machine->transaction.transaction.inLength > 0)
		{
			I2C2CONSET = _I2C2CON_SEN_MASK;
			machine->state.onEvent = &i2cMasterOnStartBitSent;
			machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
			machine->state.timeout = WAITING_FOR_START_BIT_SENT_TIMEOUT;
		}
		else
		{
			I2C2CONSET = _I2C2CON_PEN_MASK;
			machine->state.onEvent = &i2cMasterOnStopBitSent;
			machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
			machine->state.timeout = WAITING_FOR_STOP_BIT_SENT_TIMEOUT;
		}
	}
	else
		I2C2TRN = *machine->transaction.outPtr;
}

static void i2cMasterOnByteReceived(struct I2cMasterStateMachine *machine)
{
	machine->transaction.canRecoverFromCollision = false;
	if (I2C2STAT & _I2C2STAT_BCL_MASK)
		machine->transaction.transaction.statusFlags.bits.collision = 1;

	if (I2C2STAT & _I2C2STAT_RBF_MASK)
	{
		*(machine->transaction.inPtr++) = I2C2RCV;
		machine->transaction.transaction.inLength--;
	}

	I2C2CONCLR = _I2C2CON_ACKDT_MASK;
	if (machine->transaction.transaction.inChunkLength > 0)
	{
		if (++machine->transaction.inChunkArgs.length == machine->transaction.transaction.inChunkLength ||
			machine->transaction.transaction.inLength == 0 ||
			machine->transaction.transaction.statusFlags.bits.collision)
		{
			i2cMasterDispatchChunkReceived(machine);
			if (machine->transaction.inChunkArgs.nack)
				I2C2CONSET = _I2C2CON_ACKDT_MASK;

			machine->transaction.inPtr = machine->transaction.transaction.flags.bits.isInBufferDelegate
				? machine->transaction.transaction.in.asDelegate()
				: machine->transaction.transaction.in.asDirect;

			machine->transaction.inChunkArgs.chunkNumber++;
			machine->transaction.inChunkArgs.length = 0;

			if (machine->transaction.transaction.statusFlags.bits.collision)
			{
				i2cMasterOnBusCollision(machine);
				return;
			}
		}
	}

	if (machine->transaction.transaction.inLength == 0)
		I2C2CONSET = _I2C2CON_ACKDT_MASK;

	I2C2CONSET = _I2C2CON_ACKEN_MASK;

	machine->state.onEvent = &i2cMasterOnAckd;
	machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
	machine->state.timeout = machine->configuration.timeouts.waitingForMasterAck;
}

static void i2cMasterDispatchChunkReceived(struct I2cMasterStateMachine *machine)
{
	do
	{
		machine->transaction.inChunkArgs.retry = false;
		machine->transaction.inChunkArgs.nack = false;
		machine->transaction.transaction.onChunkReceived(
			&machine->transaction.transaction,
			&machine->transaction.inChunkArgs);
		
		if (machine->transaction.inChunkArgs.retry)
		{
			vTaskDelay(1);
		}
	} while (machine->transaction.inChunkArgs.retry);
}

static void i2cMasterOnAckd(struct I2cMasterStateMachine *machine)
{
	uint32_t maskedStat = I2C2STAT & _I2C2STAT_BCL_MASK;
	if (maskedStat & _I2C2STAT_BCL_MASK)
	{
		i2cMasterOnBusCollision(machine);
		return;
	}

	if (I2C2CON & _I2C2CON_ACKDT_MASK)
	{
		I2C2CONSET = _I2C2CON_PEN_MASK;
		machine->state.onEvent = &i2cMasterOnStopBitSent;
		machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
		machine->state.timeout = WAITING_FOR_STOP_BIT_SENT_TIMEOUT;
		return;
	}

	I2C2CONSET = _I2C2CON_RCEN_MASK;
	machine->state.onEvent = &i2cMasterOnByteReceived;
	machine->state.onTimeout = &i2cMasterOnTransactionTimeout;
	machine->state.timeout = machine->configuration.timeouts.waitingForSlaveDataIn;
}

static void i2cMasterOnTransactionTimeout(struct I2cMasterStateMachine *machine)
{
	machine->transaction.transaction.statusFlags.bits.success = 0;
	machine->transaction.transaction.statusFlags.bits.timeout = 1;
	machine->transaction.transaction.statusFlags.bits.writeDone = (machine->transaction.transaction.outLength == 0) ? 1 : 0;

	if (!(I2C2STAT & _I2C2STAT_P_MASK))
		I2C2CONSET = _I2C2CON_PEN_MASK;

	if (machine->transaction.inChunkArgs.length != 0)
		i2cMasterDispatchChunkReceived(machine);

	i2cMasterDispatchDone(machine);

	i2cMasterOnInitialEvent(machine);
}
