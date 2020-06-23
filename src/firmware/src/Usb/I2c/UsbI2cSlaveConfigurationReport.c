#include <xc.h>
#include <stdint.h>

#include "../../UnlockKey.h"
#include "../../I2c.h"

#include "UsbI2cInterface.h"

#define UPDATE_FLAG 0x80
#define IMMEDIATE_FLAG 0x40

#define UPDATE_IMMEDIATELY (UPDATE_FLAG | IMMEDIATE_FLAG)

__attribute__((packed))
struct I2cSlaveConfigurationReport
{
	uint8_t id;
	uint8_t unlockKey[8];

	struct
	{
		uint8_t flags;
		uint8_t low;
	} address;

	struct
	{
		uint8_t flags;
		uint8_t low;
	} addressMask;
};

static inline void usbI2cFillOutSlaveConfigurationReportFields(struct I2cSlaveConfigurationReport *out, const struct I2cSlaveAddress address);

void usbI2cOnSlaveConfigurationReportReceived(const uint8_t *report)
{
	const struct I2cSlaveConfigurationReport *in = (const struct I2cSlaveConfigurationReport *) report;
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
		usbSendAcknowledgementReport(I2C_EP_ID, I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID, REPORT_NACK_UNLOCK_KEY);
		return;
	}

	taskENTER_CRITICAL();

	struct I2cSlaveAddress address = i2cSlaveGetAddress();
	if (in->address.flags & UPDATE_FLAG)
	{
		usbCurrentConfiguration.peripherals.i2c.add = in->address.low;
		if (in->address.flags & IMMEDIATE_FLAG)
			address.as.fields.address = in->address.low;
	}

	if (in->addressMask.flags & UPDATE_FLAG)
	{
		usbCurrentConfiguration.peripherals.i2c.msk = in->addressMask.low;
		if (in->addressMask.flags & IMMEDIATE_FLAG)
			address.as.fields.mask = in->addressMask.low;
	}

	if (
		(in->address.flags & UPDATE_IMMEDIATELY) == UPDATE_IMMEDIATELY ||
		(in->addressMask.flags & UPDATE_IMMEDIATELY) == UPDATE_IMMEDIATELY)
	{
		i2cSlaveSetAddress(address);
	}

	taskEXIT_CRITICAL();
	usbSendAcknowledgementReport(I2C_EP_ID, I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbI2cGetSlaveStoredConfigurationReport(uint8_t *report)
{
	struct I2cSlaveConfigurationReport *out = (struct I2cSlaveConfigurationReport *) report;
	out->id = I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID;
	out->unlockKey[0] = 0; out->unlockKey[1] = 0; out->unlockKey[2] = 0; out->unlockKey[3] = 0;
	out->unlockKey[4] = 0; out->unlockKey[5] = 0; out->unlockKey[6] = 0; out->unlockKey[7] = 0;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	struct I2cSlaveAddress address =
	{
		.as =
		{
			.fields =
			{
				.address = (uint8_t) usbCurrentConfiguration.peripherals.i2c.add,
				.mask = (uint8_t) usbCurrentConfiguration.peripherals.i2c.msk
			}
		}
	};

	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	usbI2cFillOutSlaveConfigurationReportFields(out, address);

	return 0;
}

static inline void usbI2cFillOutSlaveConfigurationReportFields(struct I2cSlaveConfigurationReport *out, const struct I2cSlaveAddress address)
{
	out->address.flags = 0;
	out->address.low = address.as.fields.address;

	out->addressMask.flags = 0;
	out->addressMask.low = address.as.fields.mask;
}

int16_t usbI2cGetSlaveImmediateConfigurationReport(uint8_t *report)
{
	struct I2cSlaveConfigurationReport *out = (struct I2cSlaveConfigurationReport *) report;
	out->id = I2C_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID;
	out->unlockKey[0] = 0; out->unlockKey[1] = 0; out->unlockKey[2] = 0; out->unlockKey[3] = 0;
	out->unlockKey[4] = 0; out->unlockKey[5] = 0; out->unlockKey[6] = 0; out->unlockKey[7] = 0;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	struct I2cSlaveAddress address = i2cSlaveGetAddress();
	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	usbI2cFillOutSlaveConfigurationReportFields(out, address);

	return 0;
}
