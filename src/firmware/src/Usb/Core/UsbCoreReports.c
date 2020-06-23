#include <xc.h>
#include <stdint.h>

#include "../../FreeRtos.h"

#include "UsbCoreInterface.h"

static uint8_t usbCoreOutputReportBuffers[2][41];
static int usbCoreOutputReportBufferIndex = 0;
static uint8_t *usbCoreOutputReportBuffer = usbCoreOutputReportBuffers[0];

static uint8_t usbCoreInputReportBuffers[2][41];
static int usbCoreInputReportBufferIndex = 0;

int16_t usbCoreReportLengthFor(uint8_t reportId)
{
	switch (reportId)
	{
		case CORE_STATUS_REPORT_ID:
			return 41;

		case CORE_SET_UNLOCK_KEY_REPORT_ID:
			return 17;

		case CORE_STORE_CONFIGURATION_REPORT_ID:
			return 10;

		case CORE_PIN_LAT_REPORT_ID:
			return 4;

		case CORE_PIN_STATUS_REPORT_ID:
			return 6;

		case CORE_PINS_CHANGED_RESET_REPORT_ID:
			return 3;

#ifdef ENABLE_BENCH_TESTING
		case CORE_BENCH_TESTING_REPORT_ID:
			return 5;
#endif

		default:
			if ((reportId & CORE_PIN_CONFIGURATION_REPORT_ID_MASK) == CORE_PIN_CONFIGURATION_REPORT_ID)
				return 19;

			return -1;
	}
}

uint8_t *usbCoreGetOutputReportBufferFor(uint8_t reportId)
{
	return usbCoreOutputReportBuffer;
}

int8_t usbCoreOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	BaseType_t result = xQueueSendToBackFromISR(usbCoreReportsQueue, &usbCoreOutputReportBuffer, NULL);
	usbCoreOutputReportBufferIndex ^= 1;
	usbCoreOutputReportBuffer = usbCoreOutputReportBuffers[usbCoreOutputReportBufferIndex];

	if (result != pdPASS)
		usbSendAcknowledgementReportFromIsr(endpoint, *((uint8_t *) context), REPORT_NACK_QUEUE_FULL);

	return 0;
}

uint8_t *usbCoreGetInputReportFor(uint8_t reportType, uint8_t reportId)
{
	uint8_t *report = usbCoreInputReportBuffers[usbCoreInputReportBufferIndex];
	usbCoreInputReportBufferIndex ^= 1;

	int16_t status = -1;
	switch (reportId)
	{
		case CORE_STATUS_REPORT_ID:
			status = usbCoreGetStatusReport(report);
			break;

		case CORE_PIN_STATUS_REPORT_ID:
			status = usbCoreGetPinStatusReport(report);
			break;

		default:
			if ((reportId & CORE_PIN_CONFIGURATION_REPORT_ID_MASK) == CORE_PIN_CONFIGURATION_REPORT_ID)
			{
				report[0] = reportId;
				status = usbCoreGetPinConfigurationReport(report);
			}

			break;
	}

	return status >= 0 ? report : (uint8_t *) 0;
}
