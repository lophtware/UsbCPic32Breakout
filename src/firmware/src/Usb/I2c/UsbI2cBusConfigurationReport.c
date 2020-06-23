#include <xc.h>
#include <stdint.h>

#include "../../UnlockKey.h"
#include "../../I2c.h"

#include "UsbI2cInterface.h"

#define UPDATE_FLAG 0x80
#define IMMEDIATE_FLAG 0x40

__attribute__((packed))
struct I2cBusConfigurationReport
{
	uint8_t id;
	uint8_t unlockKey[8];
	uint8_t smbusLevels;
	uint8_t slewRate;
	uint8_t holdTime;
};

void usbI2cOnBusConfigurationReportReceived(const uint8_t *report)
{
	const struct I2cBusConfigurationReport *in = (const struct I2cBusConfigurationReport *) report;
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
		usbSendAcknowledgementReport(I2C_EP_ID, I2C_BUS_CONFIGURATION_REPORT_ID, REPORT_NACK_UNLOCK_KEY);
		return;
	}

	taskENTER_CRITICAL();

	if (in->smbusLevels & UPDATE_FLAG)
	{
		if (in->smbusLevels & 0x01)
		{
			usbCurrentConfiguration.peripherals.i2c.con.bits.isSmbusLevels = 1;
			if (in->smbusLevels & IMMEDIATE_FLAG)
				i2cEnableSmbusLevels();
		}
		else
		{
			usbCurrentConfiguration.peripherals.i2c.con.bits.isSmbusLevels = 0;
			if (in->smbusLevels & IMMEDIATE_FLAG)
				i2cDisableSmbusLevels();
		}
	}

	if (in->slewRate & UPDATE_FLAG)
	{
		if (in->slewRate & 0x01)
		{
			usbCurrentConfiguration.peripherals.i2c.con.bits.isSlewRateControlDisabled = 0;
			if (in->slewRate & IMMEDIATE_FLAG)
				i2cEnableSlewRateControl();
		}
		else
		{
			usbCurrentConfiguration.peripherals.i2c.con.bits.isSlewRateControlDisabled = 1;
			if (in->slewRate & IMMEDIATE_FLAG)
				i2cDisableSlewRateControl();
		}
	}

	if (in->holdTime & UPDATE_FLAG)
	{
		if (in->holdTime & 0x01)
		{
			usbCurrentConfiguration.peripherals.i2c.con.bits.isHoldTime300ns = 1;
			if (in->holdTime & IMMEDIATE_FLAG)
				i2cSetHoldTime(I2C_HOLD_TIME_300NS);
		}
		else
		{
			usbCurrentConfiguration.peripherals.i2c.con.bits.isHoldTime300ns = 0;
			if (in->holdTime & IMMEDIATE_FLAG)
				i2cSetHoldTime(I2C_HOLD_TIME_100NS);
		}
	}

	taskEXIT_CRITICAL();
	usbSendAcknowledgementReport(I2C_EP_ID, I2C_BUS_CONFIGURATION_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbI2cGetBusConfigurationReport(uint8_t *report)
{
	struct I2cBusConfigurationReport *out = (struct I2cBusConfigurationReport *) report;
	out->id = I2C_BUS_CONFIGURATION_REPORT_ID;
	out->unlockKey[0] = 0; out->unlockKey[1] = 0; out->unlockKey[2] = 0; out->unlockKey[3] = 0;
	out->unlockKey[4] = 0; out->unlockKey[5] = 0; out->unlockKey[6] = 0; out->unlockKey[7] = 0;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	out->smbusLevels =
		(usbCurrentConfiguration.peripherals.i2c.con.bits.isSmbusLevels ? 0x02 : 0x00) |
		(i2cAreSmbusLevelsEnabled() ? 0x01 : 0x00);

	out->slewRate =
		(usbCurrentConfiguration.peripherals.i2c.con.bits.isSlewRateControlDisabled ? 0x00 : 0x02) |
		(i2cIsSlewRateControlEnabled() ? 0x01 : 0x00);

	out->holdTime =
		(usbCurrentConfiguration.peripherals.i2c.con.bits.isHoldTime300ns ? 0x02 : 0x00) |
		(i2cIsHoldTime(I2C_HOLD_TIME_300NS) ? 0x01 : 0x00);

	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);
	return 0;
}
