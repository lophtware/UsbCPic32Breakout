#include <xc.h>
#include <stdint.h>

#include "../../UnlockKey.h"

#include "UsbCoreInterface.h"

void usbCoreOnSetUnlockKeyReportReceived(const uint8_t *report)
{
	struct UnlockKey suppliedKey =
	{
		.as =
		{
			.bytes =
			{
				report[1], report[2], report[3], report[4],
				report[5], report[6], report[7], report[8]
			}
		}
	};

	if (!unlockKeyMatches(&suppliedKey))
	{
		usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_NACK_UNLOCK_KEY);
		return;
	}

	struct UnlockKey newKey =
	{
		.as =
		{
			.bytes =
			{
				report[9], report[10], report[11], report[12],
				report[13], report[14], report[15], report[16]
			}
		}
	};

	unlockKeySet(&newKey);
	usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_ACK_OK);
}
