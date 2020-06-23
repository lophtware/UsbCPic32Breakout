#include <xc.h>
#include <stdint.h>

#include "../../I2c.h"

#include "UsbI2cInterface.h"

void usbI2cOnProtectedRamAddressMaskReportReceived(const uint8_t *report)
{
	uint16_t mask = (((uint16_t) report[2]) << 8) | report[1];
	i2cRamSetProtectedAddressMask(mask);
	usbSendAcknowledgementReport(I2C_EP_ID, I2C_PROTECTED_RAM_ADDRESS_MASK_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbI2cGetProtectedRamAddressMaskReport(uint8_t *report)
{
	uint16_t mask = i2cRamGetProtectedAddressMask();
	report[0] = I2C_PROTECTED_RAM_ADDRESS_MASK_REPORT_ID;
	report[1] = (uint8_t) ((mask >> 0) & 0xff);
	report[2] = (uint8_t) ((mask >> 8) & 0xff);
	return 0;
}
