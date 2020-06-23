#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../FreeRtos.h"

#include "I2c.h"

static void i2cTaskInitialise(void);
static inline void i2cConfigurationHasChanged(void);
static inline void i2cConfigurationWillChange(void);

static struct I2cState i2cState =
{
	.forceReinitialisation = I2C_REINITIALISE,
	.master = { .state = { .timeout = portMAX_DELAY } }
};

void i2cTask(void *args)
{
	static uint32_t notified, notificationValue;
	while (true)
	{
		notified = xTaskNotifyWait(0, ~0, &notificationValue, i2cState.master.state.timeout);

		if (i2cState.forceReinitialisation != I2C_INITIALISED)
			i2cTaskInitialise();

		if (notified == pdPASS)
		{
			if ((notificationValue & I2C_STATE_MACHINE_SLAVE) && i2cState.slave.state.onEvent)
				i2cState.slave.state.onEvent(&i2cState.slave);

			if ((notificationValue & I2C_STATE_MACHINE_MASTER) && i2cState.master.state.onEvent)
				i2cState.master.state.onEvent(&i2cState.master);
		}
		else
		{
			if (i2cState.master.state.onTimeout)
				i2cState.master.state.onTimeout(&i2cState.master);
		}

		if (uxQueueMessagesWaiting(i2cMasterTransactions) > 0 && i2cState.master.state.onEvent == &i2cMasterOnInitialEvent)
			i2cState.master.state.onEvent(&i2cState.master);
	}
}

static void i2cTaskInitialise(void)
{
	i2cSlaveInitialise(&i2cState.slave);
	i2cMasterInitialise(&i2cState.master, &i2cMasterTimeouts);

	if (i2cState.forceReinitialisation == I2C_REINITIALISE_AND_ENABLE)
		I2C2CONSET = _I2C2CON_ON_MASK;

	i2cState.forceReinitialisation = I2C_INITIALISED;
}

void i2cEnable(void)
{
	i2cState.forceReinitialisation = I2C_REINITIALISE_AND_ENABLE;
	i2cConfigurationHasChanged();
}

static inline void i2cConfigurationHasChanged(void)
{
	xTaskNotify(i2cTaskHandle, I2C_STATE_MACHINE_REINITIALISE, eSetBits);
}

void i2cDisable(void)
{
	I2C2CONCLR = _I2C2CON_ON_MASK;
}

void i2cEnableSmbusLevels(void)
{
	i2cConfigurationWillChange();
	I2C2CONSET = _I2C2CON_SMEN_MASK;
	i2cConfigurationHasChanged();
}

static inline void i2cConfigurationWillChange(void)
{
	i2cState.forceReinitialisation = (i2cState.forceReinitialisation == I2C_REINITIALISE_AND_ENABLE || (I2C2CON & _I2C2CON_ON_MASK))
		? I2C_REINITIALISE_AND_ENABLE
		: I2C_REINITIALISE;

	I2C2CONCLR = _I2C2CON_ON_MASK;
}

void i2cDisableSmbusLevels(void)
{
	i2cConfigurationWillChange();
	I2C2CONCLR = _I2C2CON_SMEN_MASK;
	i2cConfigurationHasChanged();
}

bool i2cAreSmbusLevelsEnabled(void)
{
	return (I2C2CON & _I2C2CON_SMEN_MASK) ? true : false;
}

void i2cEnableSlewRateControl(void)
{
	i2cConfigurationWillChange();
	I2C2CONCLR = _I2C2CON_DISSLW_MASK;
	i2cConfigurationHasChanged();
}

void i2cDisableSlewRateControl(void)
{
	i2cConfigurationWillChange();
	I2C2CONSET = _I2C2CON_DISSLW_MASK;
	i2cConfigurationHasChanged();
}

bool i2cIsSlewRateControlEnabled(void)
{
	return (I2C2CON & _I2C2CON_DISSLW_MASK) ? false : true;
}

void i2cSetHoldTime(uint8_t isHoldTime300ns)
{
	i2cConfigurationWillChange();
	if (isHoldTime300ns)
		I2C2CONSET = _I2C2CON_SDAHT_MASK;
	else
		I2C2CONCLR = _I2C2CON_SDAHT_MASK;

	i2cConfigurationHasChanged();
}

bool i2cIsHoldTime(uint8_t isHoldTime300ns)
{
	return (
		((I2C2CON & _I2C2CON_SDAHT_MASK) != 0 && isHoldTime300ns) ||
		((I2C2CON & _I2C2CON_SDAHT_MASK) == 0 && !isHoldTime300ns))
			? true
			: false;
}

bool i2cMasterIsIdleFromIsr(void)
{
	return
		(!i2cState.master.state.onEvent || i2cState.master.state.onEvent == &i2cMasterOnInitialEvent) &&
		uxQueueMessagesWaitingFromISR(i2cMasterTransactions) == 0;
}

uint16_t i2cMasterGetBaudRate(void)
{
	return (uint16_t) I2C2BRG;
}

void i2cMasterSetBaudRate(uint16_t counter)
{
	i2cConfigurationWillChange();
	I2C2BRG = (counter < I2C_BRG_MINIMUM_COUNT) ? I2C_BRG_MINIMUM_COUNT : counter;
	i2cConfigurationHasChanged();
}

void i2cMasterGetTimeouts(struct I2cMasterTimeouts *timeouts)
{
	if (!timeouts)
		return;

	memcpy(timeouts, &i2cMasterTimeouts, sizeof(struct I2cMasterTimeouts));
}

void i2cMasterSetTimeouts(const struct I2cMasterTimeouts *timeouts)
{
	if (!timeouts)
		return;

	i2cConfigurationWillChange();
	memcpy(&i2cMasterTimeouts, timeouts, sizeof(struct I2cMasterTimeouts));
	i2cConfigurationHasChanged();
}

void i2cSlaveEnableEventBroadcast(void)
{
	i2cState.slave.configuration.isEventBroadcastEnabled = true;
}

void i2cSlaveDisableEventBroadcast(void)
{
	i2cState.slave.configuration.isEventBroadcastEnabled = false;
}

bool i2cSlaveIsEventBroadcastEnabled(void)
{
	return i2cState.slave.configuration.isEventBroadcastEnabled;
}

void i2cSlaveSetAddress(const struct I2cSlaveAddress address)
{
	i2cConfigurationWillChange();
	I2C2ADD = address.as.fields.address;
	I2C2MSK = address.as.fields.mask;
	i2cConfigurationHasChanged();
}

struct I2cSlaveAddress i2cSlaveGetAddress(void)
{
	struct I2cSlaveAddress address =
	{
		.as = { .fields = { .address = (uint8_t) I2C2ADD, .mask = (uint8_t) I2C2MSK } }
	};

	return address;
}
