#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "../../UnlockKey.h"

#include "UsbUsbInterface.h"

#define UPDATE_FLAG 0x80

__attribute__((packed))
struct UsbConfigurationNameDescriptorReport
{
	uint8_t reportId;
	uint8_t flags;
	uint8_t unicode[32];
};

void usbUsbOnConfigurationNameDescriptorReportReceived(const uint8_t *report)
{
	const struct UsbConfigurationNameDescriptorReport *in = (const struct UsbConfigurationNameDescriptorReport *) report;

	if (in->flags & UPDATE_FLAG)
	{
		if (in->unicode[0] == 0 && in->unicode[1] == 0)
		{
			usbSendAcknowledgementReport(USB_EP_ID, USB_CONFIGURATION_NAME_DESCRIPTOR_REPORT_ID, REPORT_NACK_OUT_OF_BOUNDS);
			return;
		}

		taskENTER_CRITICAL();
		memcpy(usbCurrentConfiguration.unicodeName, in->unicode, sizeof(in->unicode));
		taskEXIT_CRITICAL();
	}

	usbSendAcknowledgementReport(USB_EP_ID, USB_CONFIGURATION_NAME_DESCRIPTOR_REPORT_ID, REPORT_ACK_OK);
}

int16_t usbUsbGetConfigurationNameDescriptorReport(uint8_t *report)
{
	struct UsbConfigurationNameDescriptorReport *out = (struct UsbConfigurationNameDescriptorReport *) report;
	out->reportId = USB_CONFIGURATION_NAME_DESCRIPTOR_REPORT_ID;
	out->flags = 0;

	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	memcpy(out->unicode, usbCurrentConfiguration.unicodeName, sizeof(out->unicode));
	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	return 0;
}
