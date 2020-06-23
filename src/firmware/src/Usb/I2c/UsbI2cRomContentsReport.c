#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "../../I2c.h"

#include "UsbI2cInterface.h"

__attribute__((packed))
struct I2cRomContentsReport
{
	uint8_t reportId;
	uint8_t unlockKey[8];
	struct
	{
		uint8_t low;
		uint8_t high;
	} offset;
	struct
	{
		uint8_t low;
		uint8_t high;
	} count;
	uint8_t contents[];
};

void usbI2cOnRomContentsReportReceived(const uint8_t *report)
{
	const struct I2cRomContentsReport *in = (const struct I2cRomContentsReport *) report;
	struct UnlockKey suppliedKey =
	{
		.as =
		{
			.bytes =
			{
				in->unlockKey[0],
				in->unlockKey[1],
				in->unlockKey[2],
				in->unlockKey[3],
				in->unlockKey[4],
				in->unlockKey[5],
				in->unlockKey[6],
				in->unlockKey[7]
			}
		}
	};

	if (!unlockKeyMatches(&suppliedKey))
	{
		usbSendAcknowledgementReport(I2C_EP_ID, in->reportId, REPORT_NACK_UNLOCK_KEY);
		return;
	}

	uint16_t offset = ((uint16_t) in->offset.high << 8) | in->offset.low;
	uint16_t count = ((uint16_t) in->count.high << 8) | in->count.low;
	if ((offset + count) > I2C_ROM_SIZE_BYTES || count == 0)
	{
		usbSendAcknowledgementReport(I2C_EP_ID, in->reportId, REPORT_NACK_OUT_OF_BOUNDS);
		return;
	}

	if (!i2cRomStoreAndReset(in->contents, offset, count))
		usbSendAcknowledgementReport(I2C_EP_ID, in->reportId, REPORT_NACK_OUT_OF_BOUNDS);
}

int16_t usbI2cGetRomContentsReport(uint8_t *report)
{
	report[0] = I2C_ROM_CONTENTS_REPORT_ID;
	report[1] = 0; report[2] = 0; report[3] = 0; report[4] = 0;
	report[5] = 0; report[6] = 0; report[7] = 0; report[8] = 0;
	report[9] = 0;
	report[10] = 0;
	report[11] = (I2C_ROM_SIZE_BYTES >> 0) & 0xff;
	report[12] = (I2C_ROM_SIZE_BYTES >> 8) & 0xff;
	memcpy(&report[13], (const void *) i2cRomGetContents(), I2C_ROM_SIZE_BYTES);
	return 0;
}
