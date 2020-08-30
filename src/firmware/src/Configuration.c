#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Version.h"
#include "Fault.h"
#include "Crc32.h"
#include "FreeRtos.h"
#include "Syskey.h"
#include "Nvm.h"
#include "Pins.h"
#include "UnlockKey.h"
#include "I2c.h"
#include "Spi.h"
#include "Configuration.h"

#define NUMBER_OF_CONFIGURATIONS (sizeof(configurationNvmPage.data.asFields.configurations) / sizeof(struct Configuration))

#define PACK_PIN(bank, index) ((((bank) - 'A') << 4) | (index))

#define DEFAULT_BANK_STATE_A \
	{ \
		.ansel = DEFAULT_ANSELA_MASK, \
		.tris = DEFAULT_TRISA_MASK, \
		.lat = DEFAULT_LATA_MASK, \
		.odc = DEFAULT_ODCA_MASK, \
		.cnpu = DEFAULT_CNPUA_MASK, \
		.cnpd = DEFAULT_CNPDA_MASK, \
		.cnen0 = 0, \
		.cnen1 = 0, \
		.cnenIsContinuous = 0 \
	}

#define DEFAULT_BANK_STATE_B \
	{ \
		.ansel = DEFAULT_ANSELB_MASK, \
		.tris = DEFAULT_TRISB_MASK, \
		.lat = DEFAULT_LATB_MASK, \
		.odc = DEFAULT_ODCB_MASK, \
		.cnpu = DEFAULT_CNPUB_MASK, \
		.cnpd = DEFAULT_CNPDB_MASK, \
		.cnen0 = 0, \
		.cnen1 = 0, \
		.cnenIsContinuous = 0 \
	}

#define DEFAULT_BANK_STATE_C \
	{ \
		.ansel = DEFAULT_ANSELC_MASK, \
		.tris = DEFAULT_TRISC_MASK, \
		.lat = DEFAULT_LATC_MASK, \
		.odc = DEFAULT_ODCC_MASK, \
		.cnpu = DEFAULT_CNPUC_MASK, \
		.cnpd = DEFAULT_CNPDC_MASK, \
		.cnen0 = 0, \
		.cnen1 = 0, \
		.cnenIsContinuous = 0 \
	}

#define DEFAULT_SPI_SLAVE_CONFIGURATION \
	{ \
		.registers = \
		{ \
			.con = 0, \
			.con2 = 0, \
			.brg = SPI_BRG_TYPICAL_1MHZ, \
		}, \
		.slaveSelect = \
		{ \
			.pin = { .handle = PIN_NONE, .isActiveHigh = false }, \
			.delayMicroseconds = 0 \
		} \
	}

#define DEFAULT_CONFIGURATION_NAMED(n) \
	{ \
		.unicodeName = n, \
		.unlockKey = { .as = { .dwords = {0} } }, \
		.currentLimitMilliamps = 100, \
		.dedicatedChargerAssumedCurrentLimitMilliamps = 100, \
		.isUsbShortedDataLineTestEnabled = false, \
		.pins = \
		{ \
			.configuration = \
			{ \
				.pinMaskMap = \
				{ \
					{ .bank = 0, .mask = 1 << 2 }, \
					{ .bank = 0, .mask = 1 << 3 }, \
					{ .bank = 0, .mask = 1 << 4 }, \
					{ .bank = 1, .mask = 1 << 0 }, \
					{ .bank = 1, .mask = 1 << 1 }, \
					{ .bank = 1, .mask = 1 << 2 }, \
					{ .bank = 1, .mask = 1 << 3 }, \
					{ .bank = 1, .mask = 1 << 4 }, \
					{ .bank = 1, .mask = 1 << 5 }, \
					{ .bank = 1, .mask = 1 << 7 }, \
					{ .bank = 2, .mask = 1 << 9 } \
				}, \
				.bankStates = \
				{ \
					DEFAULT_BANK_STATE_A, \
					DEFAULT_BANK_STATE_B, \
					DEFAULT_BANK_STATE_C \
				}, \
				.peripheralInputMap = \
				{ \
					.rpinr1 = 0, \
					.rpinr2 = 0, \
					.rpinr3 = 0, \
					.rpinr5 = 0, \
					.rpinr6 = 0, \
					.rpinr7 = 0, \
					.rpinr8 = 0, \
					.rpinr9 = 0, \
					.rpinr10 = 0, \
					.rpinr11 = 0, \
					.rpinr12 = 0 \
				}, \
				.peripheralOutputMap = \
				{ \
					.rpor0 = 0, \
					.rpor1 = 0, \
					.rpor2 = 0, \
					.rpor3 = 0, \
					.rpor4 = 0 \
				} \
			}, \
			.suspended = \
			{ \
				.bankStates = \
				{ \
					DEFAULT_BANK_STATE_A, \
					DEFAULT_BANK_STATE_B, \
					DEFAULT_BANK_STATE_C \
				} \
			}, \
			.assignments = \
			{ \
				.map = \
				PINS_ASSIGNMENTS_AS_ARRAY \
			} \
		}, \
		.peripherals = \
		{ \
			.i2c = \
			{ \
				.con = \
				{ \
					.bits = \
					{ \
						.isSlewRateControlDisabled = 0, \
						.isStrictAddressMode = 1, \
						.isI2cEnabled = 0, \
						.isHoldTime300ns = 1 \
					} \
				}, \
				.brg = I2C_BRG_NO_MORE_THAN_100KHZ, \
				.add = I2C_DEFAULT_ADDRESS, \
				.msk = 0x00, \
				.master = \
				{ \
					.timeouts = \
					{ \
						.waitingForAddressAck = pdMS_TO_TICKS(100), \
						.waitingForStopBit = pdMS_TO_TICKS(100), \
						.waitingForSlaveDataAck = pdMS_TO_TICKS(100), \
						.waitingForSlaveDataIn = pdMS_TO_TICKS(100), \
						.waitingForMasterAck = pdMS_TO_TICKS(100) \
					} \
				}, \
				.slave = \
				{ \
					.rom = \
					{ \
						.protectedAddressMask = 0x83ff, \
						.flags = \
						{ \
							.bits = \
							{ \
								.isWriteProtected = 1 \
							} \
						} \
					}, \
					.ram = \
					{ \
						.protectedAddressMask = 0x0000, \
						.flags = \
						{ \
							.bits = \
							{ \
								.isWriteProtected = 0 \
							} \
						} \
					}, \
					.isEventBroadcastEnabled = false \
				} \
			}, \
			.spi = \
			{ \
				.slaves = \
				{ \
					DEFAULT_SPI_SLAVE_CONFIGURATION, \
					DEFAULT_SPI_SLAVE_CONFIGURATION, \
					DEFAULT_SPI_SLAVE_CONFIGURATION, \
					DEFAULT_SPI_SLAVE_CONFIGURATION, \
					DEFAULT_SPI_SLAVE_CONFIGURATION, \
					DEFAULT_SPI_SLAVE_CONFIGURATION, \
					DEFAULT_SPI_SLAVE_CONFIGURATION, \
					DEFAULT_SPI_SLAVE_CONFIGURATION \
				} \
			} \
		} \
	}

#define DEFAULT_CONFIGURATION(n) DEFAULT_CONFIGURATION_NAMED("C\0o\0n\0f\0i\0g\0u\0r\0a\0t\0i\0o\0n\0 \0" #n)

struct ConfigurationNvmPage
{
	union
	{
		uint64_t asDwords[4096 / 8 - 1];
		struct
		{
			struct Configuration configurations[4];
			uint8_t bootConfigurationIndex;
		} asFields;
	} data;
	uint32_t firmwareVersion;
	uint32_t crc32;
};

__attribute__((section(".configuration"), aligned(2048)))
static volatile const struct ConfigurationNvmPage configurationNvmPage =
{
	.data =
	{
		.asFields =
		{
			.configurations =
			{
				DEFAULT_CONFIGURATION(1),
				DEFAULT_CONFIGURATION(2),
				DEFAULT_CONFIGURATION(3),
				DEFAULT_CONFIGURATION(4)
			},
			.bootConfigurationIndex = 0
		}
	},
	.firmwareVersion = VERSION_FIRMWARE_AS_WORD,

#ifndef BOARD_VARIANT_LITE
	.crc32 = 0x12291970ul
#else
	.crc32 = 0xac84c1e1ul
#endif
};

static const struct Configuration configurationSafeMode = DEFAULT_CONFIGURATION_NAMED("<\0!\0 \0S\0A\0F\0E\0 \0M\0O\0D\0E\0 \0!\0>\0");

__attribute__((aligned(8)))
static uint32_t configurationCalculatedCrc32;
static bool configurationIsInSafeMode;

static volatile const struct Configuration *configurationGetBootable(void);
static uint32_t configurationCalculateCrc32Dword(void);
static inline void *getHeapSection(void);
static inline uint32_t getHeapSectionSize(void);

void configurationInitialise(void)
{
	const struct Configuration *bootConfiguration = (const struct Configuration *) configurationGetBootable();
	configurationIsInSafeMode = (bootConfiguration == &configurationSafeMode);

	pinsApplyBootConfiguration(&bootConfiguration->pins.configuration);
	unlockKeySet(&bootConfiguration->unlockKey);
}

static volatile const struct Configuration *configurationGetBootable(void)
{
	bool isConfigurationNvmPageSizedCorrectly = (sizeof(configurationNvmPage) == 4096);
	bool isConfigurationNvmPageAlignedCorrectly = ((((uint32_t) &configurationNvmPage) & 2047) == 0);
	bool isHeapSizedAdequately = (getHeapSectionSize() >= 6 * 1024);

	if (isConfigurationNvmPageSizedCorrectly && isHeapSizedAdequately)
		configurationCalculatedCrc32 = configurationCalculateCrc32Dword();

	if (RESET_REASON_NEEDS_SAFEMODE(faultGetResetReason()))
		return &configurationSafeMode;

	if (!isConfigurationNvmPageSizedCorrectly)
		faultReset(RESET_REASON_INVALID_CONFIGURATION(0x01));

	if (!isConfigurationNvmPageAlignedCorrectly)
		faultReset(RESET_REASON_INVALID_CONFIGURATION(0x02));

	if (!isHeapSizedAdequately)
		faultReset(RESET_REASON_INVALID_CONFIGURATION(0x03));

	if (configurationCalculatedCrc32 != configurationNvmPage.crc32)
		faultReset(RESET_REASON_INVALID_CONFIGURATION(0x04));

	if (VERSION_FIRMWARE_MAJOR_MINOR_FROM(configurationNvmPage.firmwareVersion) != VERSION_FIRMWARE_MAJOR_MINOR_FROM(VERSION_FIRMWARE_AS_WORD))
		faultReset(RESET_REASON_INVALID_CONFIGURATION(0x05));

	return &configurationNvmPage.data.asFields.configurations[configurationNvmPage.data.asFields.bootConfigurationIndex];
}

static inline uint32_t getHeapSectionSize(void)
{
	return __builtin_section_size(".heap");
}

static uint32_t configurationCalculateCrc32Dword(void)
{
	uint32_t *crcTable = (uint32_t *) getHeapSection();
	crc32CalculateTable(crcTable);

	return crc32Calculate(
		crcTable,
		&configurationNvmPage.data,
		sizeof(configurationNvmPage.data));
}

#define ALIGNED(x, y) ((((uint32_t) (x)) + ((y) - 1)) & ~((y) - 1))
static inline void *getHeapSection(void)
{
	return (void *) ALIGNED(__builtin_section_begin(".heap"), 8);
}

uint64_t configurationGetCrc32s(void)
{
	return ((uint64_t) configurationCalculatedCrc32 << 32) | (configurationNvmPage.crc32 & 0xffffffffu);
}

void configurationApply(const struct Configuration *configuration)
{
	if (!configuration)
		return;

	pinsApplyConfiguration(&configuration->pins.configuration);
	i2cApplyConfiguration(&configuration->peripherals.i2c);
	spiApplyConfiguration(&configuration->peripherals.spi);
	unlockKeySet(&configuration->unlockKey);
}

void configurationLoad(struct Configuration *configuration, uint8_t index)
{
	if (!configuration)
		return;

	if (configurationIsInSafeMode)
	{
		memcpy(configuration, &configurationSafeMode, sizeof(struct Configuration));
		return;
	}

	if (index >= NUMBER_OF_CONFIGURATIONS)
		index = configurationNvmPage.data.asFields.bootConfigurationIndex;

	memcpy(configuration, (const void *) &configurationNvmPage.data.asFields.configurations[index], sizeof(struct Configuration));
}

bool configurationStore(const struct Configuration *configuration, uint8_t index, bool makeBoot)
{
	if (!configuration || index >= NUMBER_OF_CONFIGURATIONS || configurationIsInSafeMode)
		return false;

	__builtin_disable_interrupts();
	static const struct Configuration *staticConfiguration;
	static uint32_t staticIndex;
	static bool staticMakeBoot;
	staticConfiguration = configuration;
	staticIndex = index;
	staticMakeBoot = makeBoot;

	/*** THIS FUNCTION'S STACK PROBABLY RESIDES INSIDE THE FREERTOS HEAP, WHICH IS ABOUT TO BE HORRIBLY MANGLED...AVOID LOCAL VARIABLES AFTER THIS !!! ***/
	RESET_STACK_POINTER_TO_TOP();

	static uint8_t *heap;
	heap = (uint8_t *) getHeapSection();
	static struct ConfigurationNvmPage *configurationInHeap;
	configurationInHeap = (struct ConfigurationNvmPage *) ALIGNED(heap, 8);

	static uint32_t *crc32Table;
	crc32Table = (uint32_t *) ALIGNED(configurationInHeap + 1, 4);
	crc32CalculateTable(crc32Table);

	memcpy(configurationInHeap, (const void *) &configurationNvmPage, sizeof(struct ConfigurationNvmPage));
	memcpy(&configurationInHeap->data.asFields.configurations[staticIndex], staticConfiguration, sizeof(struct Configuration));
	if (staticMakeBoot)
		configurationInHeap->data.asFields.bootConfigurationIndex = staticIndex;

	static const uint8_t numberOfPages = (sizeof(configurationNvmPage.data) + 2047) / 2048;
	configurationInHeap->crc32 = crc32Calculate(crc32Table, configurationInHeap, sizeof(configurationNvmPage.data));
	if (nvmErasePages2048(&configurationNvmPage, numberOfPages) != numberOfPages)
		faultReset(RESET_REASON_INVALID_CONFIGURATION(0x10));

	static const uint8_t *nvmPtr;
	static const uint8_t *tooFar;
	static uint8_t *heapPtr;
	nvmPtr = (const uint8_t *) &configurationNvmPage;
	tooFar = (const uint8_t *) (&configurationNvmPage + 1);
	heapPtr = (uint8_t *) configurationInHeap;
	while (nvmPtr < tooFar)
	{
		nvmWriteRow256(nvmPtr, heapPtr);
		nvmPtr += 256;
		heapPtr += 256;
	}

	faultReset(RESET_REASON_CONFIGURATION_UPDATED);
	return true;
}

uint8_t configurationGetBootIndex(void)
{
	if (configurationIsInSafeMode)
		return 0;

	return configurationNvmPage.data.asFields.bootConfigurationIndex;
}
