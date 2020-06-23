#include <xc.h>
#include <stdint.h>

#include "../../UnlockKey.h"
#include "../../Configuration.h"

#include "UsbCoreInterface.h"

#define MAKE_BOOT_FLAG 0x80

void usbCoreOnStoreConfigurationReportReceived(const uint8_t *report)
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

	uint8_t index = usb_get_configuration() - 1;
	if (!configurationStore(&usbCurrentConfiguration, index, (report[9] & MAKE_BOOT_FLAG) ? true: false))
		usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_NACK_INVALID_CONFIGURATION);
	else
		usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_ACK_OK);
}
