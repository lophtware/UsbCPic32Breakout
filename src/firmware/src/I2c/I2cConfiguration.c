#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../FreeRtos.h"

#include "I2c.h"

struct I2cMasterTimeouts i2cMasterTimeouts =
{
	.waitingForAddressAck = pdMS_TO_TICKS(100),
	.waitingForStopBit = pdMS_TO_TICKS(100),
	.waitingForSlaveDataAck = pdMS_TO_TICKS(100),
	.waitingForSlaveDataIn = pdMS_TO_TICKS(100),
	.waitingForMasterAck = pdMS_TO_TICKS(100)
};

void i2cApplyConfiguration(const struct I2cConfiguration *i2c)
{
	if (!i2c)
		return;

	I2C2CONCLR = _I2C2CON_ON_MASK;
	IFS2CLR = _IFS2_I2C2MIF_MASK | _IFS2_I2C2SIF_MASK | _IFS2_I2C2BCIF_MASK;
	IEC2SET = _IEC2_I2C2MIE_MASK | _IEC2_I2C2SIE_MASK | _IEC2_I2C2BCIE_MASK;

	I2C2BRG = i2c->brg;
	I2C2ADD = i2c->add;
	I2C2MSK = i2c->msk;
	I2C2CON = _I2C2CON_STREN_MASK | _I2C2CON_PCIE_MASK | i2c->con.raw;

	memcpy(&i2cMasterTimeouts, &i2c->master.timeouts, sizeof(struct I2cMasterTimeouts));

	i2cRomInitialise();

	if (i2c->slave.isEventBroadcastEnabled)
		i2cSlaveEnableEventBroadcast();
	else
		i2cSlaveDisableEventBroadcast();

	i2cRomSetProtectedAddressMask(i2c->slave.rom.protectedAddressMask);
	if (i2c->slave.rom.flags.bits.isWriteProtected)
		i2cRomEnableWriteProtection();
	else
		i2cRomDisableWriteProtection();

	i2cRamInitialise();
	i2cRamSetProtectedAddressMask(i2c->slave.ram.protectedAddressMask);
	if (i2c->slave.ram.flags.bits.isWriteProtected)
		i2cRamEnableWriteProtection();
	else
		i2cRamDisableWriteProtection();
}
