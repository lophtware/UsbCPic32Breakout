#include <xc.h>
#include <stdint.h>

#include "../Fault.h"
#include "../FreeRtos.h"

#include "I2c.h"

#if configPERIPHERAL_CLOCK_HZ != 24000000
#error The value of the I2C2BRG register will be wrong if the peripheral clock is not 24MHz
#endif

extern void __attribute__((interrupt(), vector(_I2C2_MASTER_VECTOR), nomips16)) _i2c2MasterInterrupt(void);
extern void __attribute__((interrupt(), vector(_I2C2_SLAVE_VECTOR), nomips16)) _i2c2SlaveInterrupt(void);
extern void __attribute__((interrupt(), vector(_I2C2_BUS_VECTOR), nomips16)) _i2c2CollisionInterrupt(void);

TaskHandle_t i2cTaskHandle;
QueueHandle_t i2cEvents;
QueueHandle_t i2cMasterTransactions;
const struct I2cBank *i2cFirstBank, *i2cLastBank;

__attribute__((section(".i2c_nvm"), aligned(2048)))
volatile const union I2cNvmPage i2cNvmPage =
{
	.data =
	{
		.ramInitialisation = {0},
		.rom =
		{
			.asBytes =
			{
#include "DefaultRomContents.txt"
			}
		}
	}
};

void i2cInitialise(const struct I2cBank *banks, uint8_t numberOfBanks, QueueHandle_t eventQueue)
{
	if (!banks || numberOfBanks == 0)
		faultUnrecoverableInitialisationError();

	if (!eventQueue)
		faultUnrecoverableInitialisationError();

	if ((((uint32_t) &i2cNvmPage) & 2047) || sizeof(i2cNvmPage) != 2048)
		faultUnrecoverableInitialisationError();

	if (getHeapSectionSize() < 4096)
		faultUnrecoverableInitialisationError();

	i2cFirstBank = banks;
	i2cLastBank = banks + numberOfBanks;
	i2cEvents = eventQueue;

	const int priority = 3;
	IPC17bits.I2C2MIP = priority;
	IPC17bits.I2C2MIS = 0;
	IPC17bits.I2C2SIP = priority;
	IPC17bits.I2C2SIS = 0;
	IPC17bits.I2C2BCIP = priority;
	IPC17bits.I2C2BCIS = 0;

	xTaskCreate(&i2cTask, "I2C", RESERVE_STACK_USAGE_BYTES(632), NULL, priority, &i2cTaskHandle);
	i2cMasterTransactions = xQueueCreate(4, sizeof(struct I2cMasterTransaction));

	i2cRomInitialise();
	i2cRamInitialise();
}
