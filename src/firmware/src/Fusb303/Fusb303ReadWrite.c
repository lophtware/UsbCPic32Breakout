#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "Fusb303.h"

bool fusb303TryReadRegister(uint8_t registerAddress, Fusb303TransactionOnDone onDone)
{
	return fusb303TryReadRegisterWithContext(registerAddress, onDone, (void *) 0);
}

bool fusb303TryReadRegisterWithContext(uint8_t registerAddress, Fusb303TransactionOnDone onDone, void *context)
{
	struct Fusb303Transaction transaction =
	{
		.deviceAddress = FUSB303_I2C_READ | FUSB303_I2C_ADDRESS,
		.reg = { .address = registerAddress, .value = 0 },
		.flags = { .all = 0 },
		.onDone = onDone,
		.context = context
	};

	return xQueueSendToBack(fusb303Transactions, &transaction, 1) == pdPASS;
}

bool fusb303TryWriteRegister(uint8_t registerAddress, uint8_t value, Fusb303TransactionOnDone onDone)
{
	return fusb303TryWriteRegisterWithContext(registerAddress, value, onDone, (void *) 0);
}

bool fusb303TryWriteRegisterWithContext(uint8_t registerAddress, uint8_t value, Fusb303TransactionOnDone onDone, void *context)
{
	struct Fusb303Transaction transaction =
	{
		.deviceAddress = FUSB303_I2C_WRITE | FUSB303_I2C_ADDRESS,
		.reg = { .address = registerAddress, .value = value },
		.flags = { .all = 0 },
		.onDone = onDone,
		.context = context
	};

	return xQueueSendToBack(fusb303Transactions, &transaction, 1) == pdPASS;
}
