#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "Fault.h"
#include "Oscillator.h"
#include "Syskey.h"
#include "FreeRtos.h"
#include "Pins.h"
#include "Dma.h"
#include "Configuration.h"
#include "Adc.h"
#include "UsbCc.h"
#include "I2c.h"
#include "Spi.h"
#include "Usb.h"
#include "BenchTesting.h"

#include "DeviceInfoI2cBank.h"

static void initialise(void);
static void turnOffUnusedModules(void);

__attribute__((section(".heap"), aligned(8)))
uint8_t ucHeap[configTOTAL_HEAP_SIZE];

int main(void)
{
	faultGetResetReason();

	initialise();
	vTaskStartScheduler();
	return 0;
}

static void initialise(void)
{
	static const struct I2cBank i2cBanks[] =
	{
		{
			.number = 0,
			.readByte = &i2cRamReadByte,
			.writeByte = &i2cRamWriteByte
		},
		{
			.number = 1,
			.readByte = &i2cRomReadByte,
			.writeByte = &i2cRomWriteByte,
			.onTransactionDone = &i2cRomOnTransactionDone
		},
		{
			.number = 2,
			.readByte = &deviceInfoI2cBankReadByte
		},
		{
			.number = 16 + 3,
			.readByte = &usbUsbI2cBankReadByte,
			.writeByte = &usbUsbI2cBankWriteByte
		},
		{
			.number = 16 + 4,
			.readByte = &usbI2cBankReadByte,
			.writeByte = &usbI2cBankWriteByte
		}
	};

	syskeyUnlockThen(turnOffUnusedModules);
	oscillatorInitialise();
	pinsInitialise();
	dmaInitialise();
	configurationInitialise();
	adcInitialise();

	QueueHandle_t usbFlagEvents = xQueueCreate(8, sizeof(struct UsbCcEvent));
	usbCcInitialise(usbFlagEvents);

	QueueHandle_t i2cEvents = xQueueCreate(8, sizeof(struct I2cEvent));
	i2cInitialise(i2cBanks, sizeof(i2cBanks) / sizeof(const struct I2cBank), i2cEvents);

	spiInitialise();

	usbInitialise(usbFlagEvents, i2cEvents);

	benchTestingInitialise();

	PRISS = 1 << _PRISS_PRI7SS_POSITION;
}

static void turnOffUnusedModules(void)
{
	PMDCONCLR = _PMDCON_PMDLOCK_MASK;
	PMD1 = _PMD1_HLVDMD_MASK;
	PMD5 = _PMD5_U1MD_MASK | _PMD5_U3MD_MASK | _PMD5_SPI1MD_MASK | _PMD5_SPI3MD_MASK | _PMD5_I2C3MD_MASK;
	PMD6 = _PMD6_RTCCMD_MASK | _PMD6_REFOMD_MASK;
	PMDCONSET = _PMDCON_PMDLOCK_MASK;
}
