#include <xc.h>
#include <stdint.h>

#include "../FreeRtos.h"

#include "I2c.h"

void i2cMasterIsr(void)
{
	IFS2CLR = _IFS2_I2C2MIF_MASK;
	BaseType_t wasHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(i2cTaskHandle, I2C_STATE_MACHINE_MASTER, eSetBits, &wasHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(wasHigherPriorityTaskWoken);
}

void i2cCollisionIsr(void)
{
	IFS2CLR = _IFS2_I2C2BCIF_MASK;
	BaseType_t wasHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(i2cTaskHandle, I2C_STATE_MACHINE_MASTER, eSetBits, &wasHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(wasHigherPriorityTaskWoken);
}

void i2cSlaveIsr(void)
{
	IFS2CLR = _IFS2_I2C2SIF_MASK;

	BaseType_t wasHigherPriorityTaskWoken = pdFALSE;
	uint32_t flags = (I2C2STAT & _I2C2STAT_P_MASK)
		? (I2C_STATE_MACHINE_SLAVE | I2C_STATE_MACHINE_MASTER)
		: I2C_STATE_MACHINE_SLAVE;

	xTaskNotifyFromISR(i2cTaskHandle, flags, eSetBits, &wasHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(wasHigherPriorityTaskWoken);
}
