#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../FreeRtos.h"

#include "Spi.h"

#define SPICON_INTERRUPT_WHEN_TX_NOT_FULL (3 << _SPI2CON_STXISEL_POSITION)
#define SPICON_INTERRUPT_WHEN_RX_NOT_EMPTY (1 << _SPI2CON_SRXISEL_POSITION)

void spiApplyConfiguration(const struct SpiConfiguration *spi)
{
	if (!spi)
		return;

	SPI2CON = 0;
	SPI2CON2 = 0;
	while (spiState.master.flags.asBits.isBusy)
	{
		vTaskDelay(1);
	}

	memcpy(&spiState.configuration, spi, sizeof(spiState.configuration));
	for (uint8_t i = 0; i < SPI_MAXIMUM_SLAVES; i++)
		spiSlavesAssignSelectPin(i, spiState.configuration.slaves[i].slaveSelect.pin);
}
