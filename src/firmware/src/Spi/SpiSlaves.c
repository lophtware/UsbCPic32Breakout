#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "../Fault.h"
#include "../Delay.h"
#include "../FreeRtos.h"
#include "../Pins.h"

#include "Spi.h"

static void spiSlavesDeselectWithoutDelay(uint8_t index);

void spiSlavesGetConfiguration(uint8_t index, struct SpiSlaveConfiguration *configuration)
{
	if (index >= SPI_MAXIMUM_SLAVES || !configuration)
		return;

	memcpy(configuration, &spiState.configuration.slaves[index], sizeof(struct SpiSlaveConfiguration));
}

void spiSlavesSetConfiguration(uint8_t index, const struct SpiSlaveConfiguration *configuration)
{
	if (index >= SPI_MAXIMUM_SLAVES || !configuration)
		return;

	struct SpiSlaveConfiguration *dest = &spiState.configuration.slaves[index];
	memcpy(&dest->registers, &configuration->registers, sizeof(configuration->registers));

	if (dest->registers.brg < SPI_BRG_MINIMUM_COUNT)
		dest->registers.brg = SPI_BRG_MINIMUM_COUNT;
	else if (dest->registers.brg > SPI_BRG_MAXIMUM_COUNT)
		dest->registers.brg = SPI_BRG_MAXIMUM_COUNT;

	dest->slaveSelect.delayMicroseconds = configuration->slaveSelect.delayMicroseconds;
	spiSlavesAssignSelectPin(index, configuration->slaveSelect.pin);
}

void spiSlavesAssignSelectPin(uint8_t index, const struct SpiSlaveSelectPin pin)
{
	if (index >= SPI_MAXIMUM_SLAVES)
		return;

	spiState.configuration.slaves[index].slaveSelect.pin = pin;
	spiSlavesDeselectWithoutDelay(index);
}

static void spiSlavesDeselectWithoutDelay(uint8_t index)
{
	if (index >= SPI_MAXIMUM_SLAVES)
		return;

	if (spiState.configuration.slaves[index].slaveSelect.pin.isActiveHigh)
		pinsLatClear(spiState.configuration.slaves[index].slaveSelect.pin.handle);
	else
		pinsLatSet(spiState.configuration.slaves[index].slaveSelect.pin.handle);
}

void spiSlavesUnassignSelectPin(const struct Pin pin)
{
	for (uint8_t i = 0; i < SPI_MAXIMUM_SLAVES; i++)
	{
		if (pinsEqual(spiState.configuration.slaves[i].slaveSelect.pin.handle, pin))
			spiState.configuration.slaves[i].slaveSelect.pin.handle = noPin;
	}
}

void spiSlavesSelect(uint8_t index)
{
	if (index >= SPI_MAXIMUM_SLAVES)
		return;

	if (spiState.configuration.slaves[index].slaveSelect.pin.isActiveHigh)
		pinsLatSet(spiState.configuration.slaves[index].slaveSelect.pin.handle);
	else
		pinsLatClear(spiState.configuration.slaves[index].slaveSelect.pin.handle);

	delayAtLeastMicroseconds(spiState.configuration.slaves[index].slaveSelect.delayMicroseconds);
}

void spiSlavesDeselect(uint8_t index)
{
	if (index >= SPI_MAXIMUM_SLAVES)
		return;

	delayAtLeastMicroseconds(spiState.configuration.slaves[index].slaveSelect.delayMicroseconds);
	spiSlavesDeselectWithoutDelay(index);
}
