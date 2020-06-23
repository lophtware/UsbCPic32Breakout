#include <xc.h>
#include <stdint.h>

#include "../../UnlockKey.h"
#include "../../I2c.h"

#include "UsbI2cInterface.h"

#define UPDATE_FLAG 0x80
#define IMMEDIATE_FLAG 0x40

__attribute__((packed))
struct I2cTimeoutField
{
	uint8_t flags;
	uint8_t low;
	uint8_t high;
};

__attribute__((packed))
struct I2cMasterConfigurationReport
{
	uint8_t id;
	uint8_t unlockKey[8];

	struct
	{
		uint8_t flags;
		uint8_t low;
		uint8_t high;
	} baudRate;

	struct
	{
		struct I2cTimeoutField addressAck;
		struct I2cTimeoutField slaveDataAck;
		struct I2cTimeoutField slaveDataIn;
		struct I2cTimeoutField masterAck;
		struct I2cTimeoutField stopBit;
	} timeouts;
};

static inline void usbI2cFillOutMasterConfigurationReportFields(
	struct I2cMasterConfigurationReport *out,
	uint16_t baudRate,
	const struct I2cMasterTimeouts *timeouts);

void usbI2cOnMasterConfigurationReportReceived(const uint8_t *report)
{
	const struct I2cMasterConfigurationReport *in = (const struct I2cMasterConfigurationReport *) report;
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
		usbSendAcknowledgementReport(I2C_EP_ID, I2C_MASTER_STORED_CONFIGURATION_REPORT_ID, REPORT_NACK_UNLOCK_KEY);
		return;
	}

	taskENTER_CRITICAL();

	if (in->baudRate.flags & UPDATE_FLAG)
	{
		usbCurrentConfiguration.peripherals.i2c.brg = ((uint16_t) in->baudRate.high << 8) | in->baudRate.low;
		if (in->baudRate.flags & IMMEDIATE_FLAG)
			i2cMasterSetBaudRate(usbCurrentConfiguration.peripherals.i2c.brg);
	}

	static struct I2cMasterTimeouts timeouts;
	i2cMasterGetTimeouts(&timeouts);

	taskEXIT_CRITICAL();

	uint16_t timeout;
	bool timeoutsNeedImmediateUpdate = false;
	if (in->timeouts.addressAck.flags & UPDATE_FLAG)
	{
		timeout = ((uint16_t) in->timeouts.addressAck.high << 8) | in->timeouts.addressAck.low;
		usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForAddressAck = (timeout == 0) ? portMAX_DELAY : timeout;
		if (in->timeouts.addressAck.flags & IMMEDIATE_FLAG)
		{
			timeouts.waitingForAddressAck = usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForAddressAck;
			timeoutsNeedImmediateUpdate = true;
		}
	}

	if (in->timeouts.slaveDataAck.flags & UPDATE_FLAG)
	{
		timeout = ((uint16_t) in->timeouts.slaveDataAck.high << 8) | in->timeouts.slaveDataAck.low;
		usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForSlaveDataAck = (timeout == 0) ? portMAX_DELAY : timeout;
		if (in->timeouts.slaveDataAck.flags & IMMEDIATE_FLAG)
		{
			timeouts.waitingForSlaveDataAck = usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForSlaveDataAck;
			timeoutsNeedImmediateUpdate = true;
		}
	}

	if (in->timeouts.slaveDataIn.flags & UPDATE_FLAG)
	{
		timeout = ((uint16_t) in->timeouts.slaveDataIn.high << 8) | in->timeouts.slaveDataIn.low;
		usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForSlaveDataIn = (timeout == 0) ? portMAX_DELAY : timeout;
		if (in->timeouts.slaveDataIn.flags & IMMEDIATE_FLAG)
		{
			timeouts.waitingForSlaveDataIn = usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForSlaveDataIn;
			timeoutsNeedImmediateUpdate = true;
		}
	}

	if (in->timeouts.masterAck.flags & UPDATE_FLAG)
	{
		timeout = ((uint16_t) in->timeouts.masterAck.high << 8) | in->timeouts.masterAck.low;
		usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForMasterAck = (timeout == 0) ? portMAX_DELAY : timeout;
		if (in->timeouts.masterAck.flags & IMMEDIATE_FLAG)
		{
			timeouts.waitingForMasterAck = usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForMasterAck;
			timeoutsNeedImmediateUpdate = true;
		}
	}

	if (in->timeouts.stopBit.flags & UPDATE_FLAG)
	{
		timeout = ((uint16_t) in->timeouts.stopBit.high << 8) | in->timeouts.stopBit.low;
		usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForStopBit = (timeout == 0) ? portMAX_DELAY : timeout;
		if (in->timeouts.stopBit.flags & IMMEDIATE_FLAG)
		{
			timeouts.waitingForStopBit = usbCurrentConfiguration.peripherals.i2c.master.timeouts.waitingForStopBit;
			timeoutsNeedImmediateUpdate = true;
		}
	}

	if (timeoutsNeedImmediateUpdate)
	{
		taskENTER_CRITICAL();
		i2cMasterSetTimeouts(&timeouts);
		taskEXIT_CRITICAL();
	}

	usbSendAcknowledgementReport(I2C_EP_ID, I2C_MASTER_STORED_CONFIGURATION_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbI2cGetMasterStoredConfigurationReport(uint8_t *report)
{
	struct I2cMasterConfigurationReport *out = (struct I2cMasterConfigurationReport *) report;
	out->id = I2C_MASTER_STORED_CONFIGURATION_REPORT_ID;
	out->unlockKey[0] = 0; out->unlockKey[1] = 0; out->unlockKey[2] = 0; out->unlockKey[3] = 0;
	out->unlockKey[4] = 0; out->unlockKey[5] = 0; out->unlockKey[6] = 0; out->unlockKey[7] = 0;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	usbI2cFillOutMasterConfigurationReportFields(
		out,
		usbCurrentConfiguration.peripherals.i2c.brg,
		&usbCurrentConfiguration.peripherals.i2c.master.timeouts);

	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);
	return 0;
}

static inline void usbI2cFillOutMasterConfigurationReportFields(
	struct I2cMasterConfigurationReport *out,
	uint16_t baudRate,
	const struct I2cMasterTimeouts *timeouts)
{
	out->baudRate.flags = 0;
	out->baudRate.low = (uint8_t) (baudRate & 0xff);
	out->baudRate.high = (uint8_t) ((baudRate >> 8) & 0xff);

	out->timeouts.addressAck.flags = 0;
	out->timeouts.addressAck.low = (uint8_t) (timeouts->waitingForAddressAck & 0xff);
	out->timeouts.addressAck.high = (uint8_t) ((timeouts->waitingForAddressAck >> 8) & 0xff);

	out->timeouts.slaveDataAck.flags = 0;
	out->timeouts.slaveDataAck.low = (uint8_t) (timeouts->waitingForSlaveDataAck & 0xff);
	out->timeouts.slaveDataAck.high = (uint8_t) ((timeouts->waitingForSlaveDataAck >> 8) & 0xff);

	out->timeouts.slaveDataIn.flags = 0;
	out->timeouts.slaveDataIn.low = (uint8_t) (timeouts->waitingForSlaveDataIn & 0xff);
	out->timeouts.slaveDataIn.high = (uint8_t) ((timeouts->waitingForSlaveDataIn >> 8) & 0xff);

	out->timeouts.masterAck.flags = 0;
	out->timeouts.masterAck.low = (uint8_t) (timeouts->waitingForMasterAck & 0xff);
	out->timeouts.masterAck.high = (uint8_t) ((timeouts->waitingForMasterAck >> 8) & 0xff);

	out->timeouts.stopBit.flags = 0;
	out->timeouts.stopBit.low = (uint8_t) (timeouts->waitingForStopBit & 0xff);
	out->timeouts.stopBit.high = (uint8_t) ((timeouts->waitingForStopBit >> 8) & 0xff);
}

int16_t usbI2cGetMasterImmediateConfigurationReport(uint8_t *report)
{
	struct I2cMasterConfigurationReport *out = (struct I2cMasterConfigurationReport *) report;
	out->id = I2C_MASTER_STORED_CONFIGURATION_REPORT_ID;
	out->unlockKey[0] = 0; out->unlockKey[1] = 0; out->unlockKey[2] = 0; out->unlockKey[3] = 0;
	out->unlockKey[4] = 0; out->unlockKey[5] = 0; out->unlockKey[6] = 0; out->unlockKey[7] = 0;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();

	static uint16_t baudRate;
	baudRate = i2cMasterGetBaudRate();

	static struct I2cMasterTimeouts timeouts;
	i2cMasterGetTimeouts(&timeouts);

	usbI2cFillOutMasterConfigurationReportFields(out, baudRate, &timeouts);

	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);
	return 0;
}
