#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../../UnlockKey.h"
#include "../../Spi.h"

#include "UsbSpiInterface.h"

#define UPDATE_FLAG 0x80
#define IMMEDIATE_FLAG 0x40

#define UPDATE_IMMEDIATELY (UPDATE_FLAG | IMMEDIATE_FLAG)

__attribute__((packed))
struct SpiSlaveConfigurationReport
{
	uint8_t id;
	uint8_t unlockKey[8];

	struct
	{
		uint8_t flags;
		uint8_t low;
		uint8_t high;
	} baudRate;

	uint8_t clockFlags;
	uint8_t modeFlags;
	uint8_t misoFlags;

	struct
	{
		uint8_t flags;
		uint8_t counter;
	} frameSynchronisation;

	uint8_t audioFlags;
};

static inline void usbSpiFillOutSlaveConfigurationReportFields(struct SpiSlaveConfigurationReport *out, const struct SpiSlaveConfiguration *config);

void usbSpiOnSlaveConfigurationReportReceived(const uint8_t *report)
{
	const struct SpiSlaveConfigurationReport *in = (const struct SpiSlaveConfigurationReport *) report;
	struct UnlockKey suppliedKey =
	{
		.as =
		{
			.bytes =
			{
				in->unlockKey[0], in->unlockKey[1], in->unlockKey[2], in->unlockKey[3],
				in->unlockKey[4], in->unlockKey[5], in->unlockKey[6], in->unlockKey[7]
			}
		}
	};

	if (!unlockKeyMatches(&suppliedKey))
	{
		usbSendAcknowledgementReport(SPI_EP_ID, in->id, REPORT_NACK_UNLOCK_KEY);
		return;
	}

	if ((in->frameSynchronisation.flags & UPDATE_FLAG) && in->frameSynchronisation.counter >= 6)
	{
		usbSendAcknowledgementReport(SPI_EP_ID, in->id, REPORT_NACK_OUT_OF_BOUNDS);
		return;
	}

	bool hasImmediateConfigurationChanged = false;
	uint8_t slave = (in->id >> 1) & 0x07;
	struct SpiSlaveConfiguration *storedConfiguration = &usbCurrentConfiguration.peripherals.spi.slaves[slave];
	struct SpiSlaveConfiguration immediateConfiguration;

	taskENTER_CRITICAL();
	spiSlavesGetConfiguration(slave, &immediateConfiguration);

	if (in->baudRate.flags & UPDATE_FLAG)
	{
		uint16_t brg = ((uint16_t) in->baudRate.high << 8) | in->baudRate.low;
		if (brg < SPI_BRG_MINIMUM_COUNT)
			brg = SPI_BRG_MINIMUM_COUNT;
		else if (brg > SPI_BRG_MAXIMUM_COUNT)
			brg = SPI_BRG_MAXIMUM_COUNT;

		storedConfiguration->registers.brg = brg;
		if (in->baudRate.flags & IMMEDIATE_FLAG)
		{
			immediateConfiguration.registers.brg = brg;
			hasImmediateConfigurationChanged = true;
		}
	}

	if (in->clockFlags & UPDATE_FLAG)
	{
		storedConfiguration->registers.con &= ~(_SPI2CON_CKP_MASK | _SPI2CON_CKE_MASK);
		storedConfiguration->registers.con |=
			((in->clockFlags & 0x02) ? _SPI2CON_CKP_MASK : 0x00) |
			((in->clockFlags & 0x01) ? _SPI2CON_CKE_MASK : 0x00);

		if (in->clockFlags & IMMEDIATE_FLAG)
		{
			immediateConfiguration.registers.con &= ~(_SPI2CON_CKP_MASK | _SPI2CON_CKE_MASK);
			immediateConfiguration.registers.con |=
				((in->clockFlags & 0x02) ? _SPI2CON_CKP_MASK : 0x00) |
				((in->clockFlags & 0x01) ? _SPI2CON_CKE_MASK : 0x00);

			hasImmediateConfigurationChanged = true;
		}
	}

	if (in->modeFlags & UPDATE_FLAG)
	{
		storedConfiguration->registers.con &= ~(_SPI2CON_MODE16_MASK | _SPI2CON_MODE32_MASK);
		storedConfiguration->registers.con |=
			((in->modeFlags & 0x02) ? _SPI2CON_MODE32_MASK : 0x00) |
			((in->modeFlags & 0x01) ? _SPI2CON_MODE16_MASK : 0x00);

		if (in->modeFlags & IMMEDIATE_FLAG)
		{
			immediateConfiguration.registers.con &= ~(_SPI2CON_MODE16_MASK | _SPI2CON_MODE32_MASK);
			immediateConfiguration.registers.con |=
				((in->modeFlags & 0x02) ? _SPI2CON_MODE32_MASK : 0x00) |
				((in->modeFlags & 0x01) ? _SPI2CON_MODE16_MASK : 0x00);

			hasImmediateConfigurationChanged = true;
		}
	}

	if (in->misoFlags & UPDATE_FLAG)
	{
		storedConfiguration->registers.con &= ~_SPI2CON_SMP_MASK;
		storedConfiguration->registers.con |= (in->misoFlags & 0x01) ? _SPI2CON_SMP_MASK : 0x00;

		storedConfiguration->registers.con2 &= ~_SPI2CON2_SPISGNEXT_MASK;
		storedConfiguration->registers.con2 |= (in->misoFlags & 0x02) ? _SPI2CON2_SPISGNEXT_MASK : 0x00;

		if (in->misoFlags & IMMEDIATE_FLAG)
		{
			immediateConfiguration.registers.con &= ~_SPI2CON_SMP_MASK;
			immediateConfiguration.registers.con |= (in->misoFlags & 0x01) ? _SPI2CON_SMP_MASK : 0x00;

			immediateConfiguration.registers.con2 &= ~_SPI2CON2_SPISGNEXT_MASK;
			immediateConfiguration.registers.con2 |= (in->misoFlags & 0x02) ? _SPI2CON2_SPISGNEXT_MASK : 0x00;

			hasImmediateConfigurationChanged = true;
		}
	}

	if (in->frameSynchronisation.flags & UPDATE_FLAG)
	{
		storedConfiguration->registers.con &= ~(_SPI2CON_FRMEN_MASK | _SPI2CON_FRMSYPW_MASK | _SPI2CON_FRMPOL_MASK | _SPI2CON_SPIFE_MASK | _SPI2CON_FRMCNT_MASK);
		storedConfiguration->registers.con |=
			((in->frameSynchronisation.flags & 0x08) ? _SPI2CON_FRMEN_MASK : 0x00) |
			((in->frameSynchronisation.flags & 0x04) ? _SPI2CON_FRMSYPW_MASK : 0x00) |
			((in->frameSynchronisation.flags & 0x02) ? _SPI2CON_FRMPOL_MASK : 0x00) |
			((in->frameSynchronisation.flags & 0x01) ? _SPI2CON_SPIFE_MASK : 0x00);

		storedConfiguration->registers.con |= in->frameSynchronisation.counter << _SPI2CON_FRMCNT_POSITION;

		if (in->frameSynchronisation.flags & IMMEDIATE_FLAG)
		{
			immediateConfiguration.registers.con &= ~(_SPI2CON_FRMEN_MASK | _SPI2CON_FRMSYPW_MASK | _SPI2CON_FRMPOL_MASK | _SPI2CON_SPIFE_MASK | _SPI2CON_FRMCNT_MASK);
			immediateConfiguration.registers.con |=
				((in->frameSynchronisation.flags & 0x08) ? _SPI2CON_FRMEN_MASK : 0x00) |
				((in->frameSynchronisation.flags & 0x04) ? _SPI2CON_FRMSYPW_MASK : 0x00) |
				((in->frameSynchronisation.flags & 0x02) ? _SPI2CON_FRMPOL_MASK : 0x00) |
				((in->frameSynchronisation.flags & 0x01) ? _SPI2CON_SPIFE_MASK : 0x00);

			immediateConfiguration.registers.con |= in->frameSynchronisation.counter << _SPI2CON_FRMCNT_POSITION;

			hasImmediateConfigurationChanged = true;
		}
	}

	if (in->audioFlags & UPDATE_FLAG)
	{
		storedConfiguration->registers.con2 &= ~(_SPI2CON2_AUDEN_MASK | _SPI2CON2_AUDMONO_MASK | _SPI2CON2_AUDMOD_MASK);
		storedConfiguration->registers.con2 |=
			((in->frameSynchronisation.flags & 0x08) ? _SPI2CON2_AUDEN_MASK : 0x00) |
			((in->frameSynchronisation.flags & 0x04) ? _SPI2CON2_AUDMONO_MASK : 0x00) |
			((in->frameSynchronisation.flags & 0x02) ? _SPI2CON2_AUDMOD1_MASK : 0x00) |
			((in->frameSynchronisation.flags & 0x01) ? _SPI2CON2_AUDMOD0_MASK : 0x00);

		if (in->audioFlags & IMMEDIATE_FLAG)
		{
			immediateConfiguration.registers.con2 &= ~(_SPI2CON2_AUDEN_MASK | _SPI2CON2_AUDMONO_MASK | _SPI2CON2_AUDMOD_MASK);
			immediateConfiguration.registers.con2 |=
				((in->frameSynchronisation.flags & 0x08) ? _SPI2CON2_AUDEN_MASK : 0x00) |
				((in->frameSynchronisation.flags & 0x04) ? _SPI2CON2_AUDMONO_MASK : 0x00) |
				((in->frameSynchronisation.flags & 0x02) ? _SPI2CON2_AUDMOD1_MASK : 0x00) |
				((in->frameSynchronisation.flags & 0x01) ? _SPI2CON2_AUDMOD0_MASK : 0x00);

			hasImmediateConfigurationChanged = true;
		}
	}

	if (hasImmediateConfigurationChanged)
		spiSlavesSetConfiguration(slave, &immediateConfiguration);

	taskEXIT_CRITICAL();
	usbSendAcknowledgementReport(SPI_EP_ID, in->id, REPORT_ACK_OK);
}

int16_t usbSpiGetSlaveStoredConfigurationReport(uint8_t *report)
{
	struct SpiSlaveConfigurationReport *out = (struct SpiSlaveConfigurationReport *) report;
	out->unlockKey[0] = 0; out->unlockKey[1] = 0; out->unlockKey[2] = 0; out->unlockKey[3] = 0;
	out->unlockKey[4] = 0; out->unlockKey[5] = 0; out->unlockKey[6] = 0; out->unlockKey[7] = 0;

	uint8_t slave = (out->id >> 1) & 0x07;
	struct SpiSlaveConfiguration storedConfiguration;
	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	memcpy(&storedConfiguration, &usbCurrentConfiguration.peripherals.spi.slaves[slave], sizeof(struct SpiSlaveConfiguration));
	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	usbSpiFillOutSlaveConfigurationReportFields(out, &storedConfiguration);

	return 0;
}

static inline void usbSpiFillOutSlaveConfigurationReportFields(struct SpiSlaveConfigurationReport *out, const struct SpiSlaveConfiguration *config)
{
	out->baudRate.flags = 0;
	out->baudRate.low = (uint8_t) (config->registers.brg & 0xff);
	out->baudRate.high = (uint8_t) ((config->registers.brg >> 8) & 0xff);

	out->clockFlags =
		((config->registers.con & _SPI2CON_CKP_MASK) ? 0x02 : 0x00) |
		((config->registers.con & _SPI2CON_CKE_MASK) ? 0x01 : 0x00);

	out->modeFlags =
		((config->registers.con & _SPI2CON_MODE32_MASK) ? 0x02 : 0x00) |
		((config->registers.con & _SPI2CON_MODE16_MASK) ? 0x01 : 0x00);

	out->misoFlags =
		((config->registers.con2 & _SPI2CON2_SPISGNEXT_MASK) ? 0x02 : 0x00) |
		((config->registers.con & _SPI2CON_SMP_MASK) ? 0x01 : 0x00);

	out->frameSynchronisation.flags =
		((config->registers.con & _SPI2CON_FRMEN_MASK) ? 0x08 : 0x00) |
		((config->registers.con & _SPI2CON_FRMSYPW_MASK) ? 0x04 : 0x00) |
		((config->registers.con & _SPI2CON_FRMPOL_MASK) ? 0x02 : 0x00) |
		((config->registers.con & _SPI2CON_SPIFE_MASK) ? 0x01 : 0x00);

	out->frameSynchronisation.counter = (uint8_t) ((config->registers.con >> _SPI2CON_FRMCNT_POSITION) & 0x07);

	out->audioFlags =
		((config->registers.con2 & _SPI2CON2_AUDEN_MASK) ? 0x08 : 0x00) |
		((config->registers.con2 & _SPI2CON2_AUDMONO_MASK) ? 0x04 : 0x00) |
		((config->registers.con2 & _SPI2CON2_AUDMOD1_MASK) ? 0x02 : 0x00) |
		((config->registers.con2 & _SPI2CON2_AUDMOD0_MASK) ? 0x01 : 0x00);
}

int16_t usbSpiGetSlaveImmediateConfigurationReport(uint8_t *report)
{
	struct SpiSlaveConfigurationReport *out = (struct SpiSlaveConfigurationReport *) report;
	out->unlockKey[0] = 0; out->unlockKey[1] = 0; out->unlockKey[2] = 0; out->unlockKey[3] = 0;
	out->unlockKey[4] = 0; out->unlockKey[5] = 0; out->unlockKey[6] = 0; out->unlockKey[7] = 0;

	uint8_t slave = (out->id >> 1) & 0x07;
	struct SpiSlaveConfiguration immediateConfiguration;
	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	spiSlavesGetConfiguration(slave, &immediateConfiguration);
	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	usbSpiFillOutSlaveConfigurationReportFields(out, &immediateConfiguration);

	return 0;
}
