#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../FreeRtos.h"
#include "../../Spi.h"

#include "UsbSpiInterface.h"

QueueHandle_t usbSpiReportsQueue;

void usbSpiReportsTask(void *args)
{
	while (true)
	{
		static const uint8_t *report;
		xQueueReceive(usbSpiReportsQueue, &report, portMAX_DELAY);
		if (!report)
			continue;

		switch (report[0])
		{
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(0):
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(1):
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(2):
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(3):
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(4):
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(5):
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(6):
			case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(7):
				usbSpiOnSlaveConfigurationReportReceived(report);
				break;

			default:
				if (report[0] & SPI_TRANSACTION_REPORT_FLAG)
					usbSpiOnTransactionReportReceived(report);
				else
					usbSendAcknowledgementReport(SPI_EP_ID, report[0], REPORT_NACK_UNKNOWN_ID);

				break;
		}
	}
}
