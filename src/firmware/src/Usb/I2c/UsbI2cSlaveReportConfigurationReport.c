#include <xc.h>
#include <stdint.h>

#include "../../FreeRtos.h"
#include "../../I2c.h"

#include "UsbI2cInterface.h"

#define TRANSACTION_NOTIFICATION_FLAG 0x01
#define DEFAULT_TRANSACTION_NOTIFICATION_FLAG 0x02

void usbI2cOnSlaveReportConfigurationReportReceived(const uint8_t *report)
{
	if (report[1] & TRANSACTION_NOTIFICATION_FLAG)
		i2cSlaveEnableEventBroadcast();
	else
		i2cSlaveDisableEventBroadcast();

	taskENTER_CRITICAL();

	if (report[1] & DEFAULT_TRANSACTION_NOTIFICATION_FLAG)
		usbCurrentConfiguration.peripherals.i2c.slave.isEventBroadcastEnabled = 1;
	else
		usbCurrentConfiguration.peripherals.i2c.slave.isEventBroadcastEnabled = 0;

	taskEXIT_CRITICAL();
	usbSendAcknowledgementReport(I2C_EP_ID, I2C_SLAVE_REPORT_CONFIGURATION_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbI2cGetSlaveReportConfigurationReport(uint8_t *report)
{
	report[0] = I2C_SLAVE_REPORT_CONFIGURATION_REPORT_ID;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();

	report[1] =
		(i2cSlaveIsEventBroadcastEnabled() ? TRANSACTION_NOTIFICATION_FLAG : 0x00) |
		(usbCurrentConfiguration.peripherals.i2c.slave.isEventBroadcastEnabled ? DEFAULT_TRANSACTION_NOTIFICATION_FLAG : 0x00);

	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	return 0;
}
