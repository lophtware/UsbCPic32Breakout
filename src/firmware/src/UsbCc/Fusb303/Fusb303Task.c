#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../FreeRtos.h"

#include "Fusb303.h"

static void fusb303PerformRead(struct Fusb303Transaction *transaction);
static void fusb303PerformWrite(struct Fusb303Transaction *transaction);

void fusb303Task(void *args)
{
	fusb303InitialiseController();

	static struct Fusb303Transaction transaction;
	while (true)
	{
		xQueueReceive(fusb303Transactions, &transaction, portMAX_DELAY);
		if ((transaction.deviceAddress & 0x01) == FUSB303_I2C_READ)
			fusb303PerformRead(&transaction);
		else
			fusb303PerformWrite(&transaction);
	}
}

static void fusb303PerformRead(struct Fusb303Transaction *transaction)
{
	ulTaskNotifyTake(pdTRUE, 0);
	I2C1STATCLR = ~0;
	I2C1CONCLR = _I2C1CON_ACKDT_MASK;
	I2C1CONSET = _I2C1CON_SEN_MASK;
	transaction->flags.all = 0;

	while (true)
	{
		if (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(50)) == 0)
		{
			I2C1CONSET = _I2C1CON_PEN_MASK;
			break;
		}

		if (I2C1STATbits.S && !transaction->flags.bits.startBitSeen)
		{
			if (!transaction->flags.bits.isRegisterAddressAckd)
				I2C1TRN = FUSB303_I2C_WRITE | (transaction->deviceAddress & ~0x01);
			else
				I2C1TRN = FUSB303_I2C_READ | (transaction->deviceAddress & ~0x01);

			transaction->flags.bits.startBitSeen = true;
		}
		else if (I2C1STATbits.P)
		{
			break;
		}
		else if (I2C1STATbits.RBF)
		{
			I2C1CONSET = _I2C1CON_ACKDT_MASK | _I2C1CON_ACKEN_MASK;
			transaction->reg.value = I2C1RCV;
		}
		else if (I2C1CONbits.ACKDT == 1)
		{
			I2C1CONSET = _I2C1CON_PEN_MASK;
			transaction->flags.bits.success = true;
		}
		else if (transaction->flags.bits.startBitSeen && I2C1STATbits.ACKSTAT == 0)
		{
			if (!transaction->flags.bits.isDeviceAddressAckd)
			{
				I2C1TRN = transaction->reg.address;
				transaction->flags.bits.isDeviceAddressAckd = true;
			}
			else if (!transaction->flags.bits.isRegisterAddressAckd)
			{
				I2C1CONSET = _I2C1CON_RSEN_MASK;
				transaction->flags.bits.isRegisterAddressAckd = true;
				transaction->flags.bits.startBitSeen = false;
			}
			else
			{
				I2C1CONSET = _I2C1CON_RCEN_MASK;
			}
		}
		else
		{
			I2C1CONSET = _I2C1CON_PEN_MASK;
		}
	}

	if (transaction->onDone)
		transaction->onDone(transaction);
}

static void fusb303PerformWrite(struct Fusb303Transaction *transaction)
{
	ulTaskNotifyTake(pdTRUE, 0);
	I2C1STATCLR = ~0;
	I2C1CONSET = _I2C1CON_SEN_MASK;
	transaction->flags.all = 0;

	while (true)
	{
		if (ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(50)) == 0)
		{
			I2C1CONSET = _I2C1CON_PEN_MASK;
			break;
		}

		if (I2C1STATbits.S && !transaction->flags.bits.startBitSeen)
		{
			transaction->flags.bits.startBitSeen = true;
			I2C1TRN = transaction->deviceAddress;
		}
		else if (I2C1STATbits.P)
		{
			break;
		}
		else if (transaction->flags.bits.startBitSeen && I2C1STATbits.ACKSTAT == 0)
		{
			if (!transaction->flags.bits.isDeviceAddressAckd)
			{
				I2C1TRN = transaction->reg.address;
				transaction->flags.bits.isDeviceAddressAckd = true;
			}
			else if (!transaction->flags.bits.isRegisterAddressAckd)
			{
				I2C1TRN = transaction->reg.value;
				transaction->flags.bits.isRegisterAddressAckd = true;
			}
			else
			{
				I2C1CONSET = _I2C1CON_PEN_MASK;
				transaction->flags.bits.success = true;
			}
		}
		else
		{
			I2C1CONSET = _I2C1CON_PEN_MASK;
		}
	}

	if (transaction->onDone)
		transaction->onDone(transaction);
}
