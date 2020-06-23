#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "UsbCoreInterface.h"

#include "../../FreeRtos.h"
#include "../../BenchTesting.h"

QueueHandle_t usbCoreReportsQueue;

void usbCoreReportsTask(void *args)
{
	static const uint8_t *report;
	while (true)
	{
		xQueueReceive(usbCoreReportsQueue, &report, portMAX_DELAY);
		if (!report)
			continue;

		switch (report[0])
		{
			case CORE_SET_UNLOCK_KEY_REPORT_ID:
				usbCoreOnSetUnlockKeyReportReceived(report);
				break;

			case CORE_STORE_CONFIGURATION_REPORT_ID:
				usbCoreOnStoreConfigurationReportReceived(report);
				break;

			case CORE_PIN_LAT_REPORT_ID:
				usbCoreOnPinLatReportReceived(report);
				break;

			case CORE_PINS_CHANGED_RESET_REPORT_ID:
				usbCoreOnPinsChangedResetReportReceived(report);
				break;

#ifdef ENABLE_BENCH_TESTING
			case CORE_BENCH_TESTING_REPORT_ID:
				benchTestingSendRawCommand(report[1] | ((uint32_t) report[2] << 8) | ((uint32_t) report[3] << 16) | ((uint32_t) report[4] << 24));
				break;
#endif

			default:
				if ((report[0] & CORE_PIN_CONFIGURATION_REPORT_ID_MASK) == CORE_PIN_CONFIGURATION_REPORT_ID)
					usbCoreOnPinConfigurationReportReceived(report);
				else
					usbSendAcknowledgementReport(CORE_EP_ID, report[0], REPORT_NACK_UNKNOWN_ID);

				break;
		}
	}
}
