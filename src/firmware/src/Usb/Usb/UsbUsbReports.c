#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../FreeRtos.h"

#include "UsbUsbInterface.h"

static uint8_t usbUsbOutputReportBuffers[2][34];
static int usbUsbOutputReportBufferIndex = 0;
static uint8_t *usbUsbOutputReportBuffer = usbUsbOutputReportBuffers[0];

static uint8_t usbUsbInputReportBuffers[2][34];
static int usbUsbInputReportBufferIndex = 0;

int16_t usbUsbReportLengthFor(uint8_t reportId)
{
	switch (reportId)
	{
		case USB_CONFIGURATION_NAME_DESCRIPTOR_REPORT_ID:
			return 34;

		case USB_POWER_CONFIGURATION_REPORT_ID:
			return 7;

		default:
			return -1;
	}
}

uint8_t *usbUsbGetOutputReportBufferFor(uint8_t reportId)
{
	usbUsbOutputReportBufferIndex ^= 1;
	usbUsbOutputReportBuffer = usbUsbOutputReportBuffers[usbUsbOutputReportBufferIndex];
	return usbUsbOutputReportBuffer;
}

int8_t usbUsbOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	if (xQueueSendToBackFromISR(usbUsbReportsQueue, &context, NULL) != pdPASS)
	{
		usbSendAcknowledgementReportFromIsr(endpoint, *((uint8_t *) context), REPORT_NACK_QUEUE_FULL);
		return MSTACK_STALL;
	}

	return 0;
}

uint8_t *usbUsbGetInputReportFor(uint8_t reportType, uint8_t reportId)
{
	uint8_t *report = usbUsbInputReportBuffers[usbUsbInputReportBufferIndex];
	usbUsbInputReportBufferIndex ^= 1;

	int16_t status = -1;
	switch (reportId)
	{
		case USB_CONFIGURATION_NAME_DESCRIPTOR_REPORT_ID:
			status = usbUsbGetConfigurationNameDescriptorReport(report);
			break;

		case USB_POWER_CONFIGURATION_REPORT_ID:
			status = usbUsbGetPowerConfigurationReport(report);
			break;

		default:
			break;
	}

	return status >= 0 ? report : (uint8_t *) 0;
}
