#include <xc.h>
#include <stdint.h>

#include "../../FreeRtos.h"
#include "../../I2c.h"

#include "UsbI2cInterface.h"

#define CAN_WRITE_FLAG 0x01
#define DEFAULT_CAN_WRITE_FLAG 0x02

void usbI2cOnRomBankWriteProtectFlagReportReceived(const uint8_t *report)
{
	if (report[1] & CAN_WRITE_FLAG)
		i2cRomDisableWriteProtection();
	else
		i2cRomEnableWriteProtection();

	taskENTER_CRITICAL();

	if (report[1] & DEFAULT_CAN_WRITE_FLAG)
		usbCurrentConfiguration.peripherals.i2c.slave.rom.flags.bits.isWriteProtected = 0;
	else
		usbCurrentConfiguration.peripherals.i2c.slave.rom.flags.bits.isWriteProtected = 1;

	taskEXIT_CRITICAL();
	usbSendAcknowledgementReport(I2C_EP_ID, I2C_ROM_BANK_WRITE_PROTECT_FLAG_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbI2cGetRomBankWriteProtectFlagReport(uint8_t *report)
{
	report[0] = I2C_ROM_BANK_WRITE_PROTECT_FLAG_REPORT_ID;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();

	report[1] =
		(i2cRomIsWriteProtected() ? 0x00 : CAN_WRITE_FLAG) |
		(usbCurrentConfiguration.peripherals.i2c.slave.rom.flags.bits.isWriteProtected ? 0x00 : DEFAULT_CAN_WRITE_FLAG);

	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	return 0;
}
