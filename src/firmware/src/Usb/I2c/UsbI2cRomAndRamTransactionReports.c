#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "../../I2c.h"

#include "UsbI2cInterface.h"

#define RW_FLAG 0x80
#define SUCCESS_FLAG 0x01

__attribute__((packed))
struct RomAndRamTransactionReport
{
	uint8_t reportId;
	uint8_t flags;
	struct
	{
		uint8_t low;
		uint8_t high;
	} readAddress;
	uint8_t readCount;
	struct
	{
		uint8_t low;
		uint8_t high;
	} writeAddress;
	uint8_t writeCount;
	uint8_t payload[32];
};

void usbI2cOnRomAndRamTransactionReportReceived(const uint8_t *report)
{
	const struct RomAndRamTransactionReport *in = (const struct RomAndRamTransactionReport *) report;
	if (in->readCount > 32 || in->writeCount > 32)
	{
		usbSendAcknowledgementReport(I2C_EP_ID, report[0], REPORT_NACK_OUT_OF_BOUNDS);
		return;
	}

	uint16_t readAddress = (in->readAddress.high << 8) | in->readAddress.low;
	uint16_t writeAddress = (in->writeAddress.high << 8) | in->writeAddress.low;
	while (usbIsInEndpointBusy(I2C_EP_ID))
	{
		vTaskDelay(1);
	}

	struct RomAndRamTransactionReport *out = (struct RomAndRamTransactionReport *) usb_get_in_buffer(I2C_EP_ID);
	memcpy(out, in, sizeof(struct RomAndRamTransactionReport) - sizeof(in->payload));

	if (in->flags & RW_FLAG)
	{
		if (in->reportId == I2C_ROM_TRANSACTION_REPORT_ID)
		{
			i2cRomReadBytesUnprotected(out->payload, readAddress, in->readCount);
			out->flags = RW_FLAG | (i2cRomWriteBytesUnprotected(writeAddress, in->payload, in->writeCount) ? SUCCESS_FLAG : 0);
		}
		else
		{
			i2cRamReadBytesUnprotected(out->payload, readAddress, in->readCount);
			out->flags = RW_FLAG | (i2cRamWriteBytesUnprotected(writeAddress, in->payload, in->writeCount) ? SUCCESS_FLAG : 0);
		}
	}
	else
	{
		if (in->reportId == I2C_ROM_TRANSACTION_REPORT_ID)
		{
			out->flags = i2cRomWriteBytesUnprotected(writeAddress, in->payload, in->writeCount) ? SUCCESS_FLAG : 0;
			i2cRomReadBytesUnprotected(out->payload, readAddress, in->readCount);
		}
		else
		{
			out->flags = i2cRamWriteBytesUnprotected(writeAddress, in->payload, in->writeCount) ? SUCCESS_FLAG : 0;
			i2cRamReadBytesUnprotected(out->payload, readAddress, in->readCount);
		}
	}

	usb_send_in_buffer(I2C_EP_ID, sizeof(struct RomAndRamTransactionReport));
}
