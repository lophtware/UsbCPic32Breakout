#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../FreeRtos.h"

#include "UsbSpiInterface.h"

#define SPI_TRANSACTION_REPORT_LENGTH_FROM(reportId) (((reportId) & (~SPI_TRANSACTION_REPORT_FLAG)) * 8 + 12)

uint8_t usbSpiTransactionReportBuffer[1028];

static uint8_t usbSpiOutputReportBuffers[2][18];
static int usbSpiOutputReportBufferIndex = 0;
static uint8_t *usbSpiOutputReportBuffer = usbSpiOutputReportBuffers[0];

static uint8_t usbSpiInputReportBuffers[2][18];
static int usbSpiInputReportBufferIndex = 0;

int16_t usbSpiReportLengthFor(uint8_t reportId)
{
	switch (reportId)
	{
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(0):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(1):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(2):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(3):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(4):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(5):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(6):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(7):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(0):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(1):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(2):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(3):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(4):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(5):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(6):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(7):
			return 18;

		default:
			if (reportId & SPI_TRANSACTION_REPORT_FLAG)
				return SPI_TRANSACTION_REPORT_LENGTH_FROM(reportId);

			return -1;
	}
}

uint8_t *usbSpiGetOutputReportBufferFor(uint8_t reportId)
{
	if ((reportId & SPI_TRANSACTION_REPORT_FLAG) == 0)
	{
		usbSpiOutputReportBufferIndex ^= 1;
		usbSpiOutputReportBuffer = usbSpiOutputReportBuffers[usbSpiOutputReportBufferIndex];
		return usbSpiOutputReportBuffer;
	}

	return spiMasterIsIdleFromIsr() ? usbSpiTransactionReportBuffer : (uint8_t *) 0;
}

int8_t usbSpiOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	if (xQueueSendToBackFromISR(usbSpiReportsQueue, &context, NULL) != pdPASS)
	{
		usbSendAcknowledgementReportFromIsr(endpoint, *((uint8_t *) context), REPORT_NACK_QUEUE_FULL);
		return MSTACK_STALL;
	}

	return 0;
}

uint8_t *usbSpiGetInputReportFor(uint8_t reportType, uint8_t reportId)
{
	uint8_t *report = usbSpiInputReportBuffers[usbSpiInputReportBufferIndex];
	usbSpiInputReportBufferIndex ^= 1;

	report[0] = reportId;

	int16_t status = -1;
	switch (reportId)
	{
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(0):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(1):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(2):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(3):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(4):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(5):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(6):
		case SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(7):
			status = usbSpiGetSlaveImmediateConfigurationReport(report);
			break;

		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(0):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(1):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(2):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(3):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(4):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(5):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(6):
		case SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(7):
			status = usbSpiGetSlaveStoredConfigurationReport(report);
			break;

		default:
			break;
	}

	return status >= 0 ? report : (uint8_t *) 0;
}
