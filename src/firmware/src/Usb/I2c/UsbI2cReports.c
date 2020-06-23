#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../FreeRtos.h"

#include "UsbI2cInterface.h"

#define I2C_TRANSACTION_REPORT_LENGTH_FROM(reportId) (((reportId) & (~I2C_TRANSACTION_REPORT_FLAG)) * 8 + 12)

static uint8_t usbI2cOutputTransactionReportBuffer[1042];
static uint8_t *usbI2cOutputTransactionReportBufferPtr = usbI2cOutputTransactionReportBuffer;

static uint8_t usbI2cOutputReportBuffers[2][48];
static int usbI2cOutputReportBufferIndex = 0;
static uint8_t *usbI2cOutputReportBuffer = usbI2cOutputReportBuffers[0];

static uint8_t usbI2cInputReportBuffers[2][48];
static int usbI2cInputReportBufferIndex = 0;

int16_t usbI2cReportLengthFor(uint8_t reportId)
{
	switch (reportId)
	{
		case I2C_BUS_CONFIGURATION_REPORT_ID:
			return 12;

		case I2C_MASTER_STORED_CONFIGURATION_REPORT_ID:
		case I2C_MASTER_IMMEDIATE_CONFIGURATION_REPORT_ID:
			return 27;

		case I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID:
		case I2C_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID:
			return 13;

		case I2C_SLAVE_REPORT_CONFIGURATION_REPORT_ID:
			return 2;

		case I2C_PROTECTED_RAM_ADDRESS_MASK_REPORT_ID:
			return 3;

		case I2C_RAM_BANK_WRITE_PROTECT_FLAG_REPORT_ID:
			return 2;

		case I2C_RAM_INITIALISATION_CONTENTS_REPORT_ID:
			return 1037;

		case I2C_PROTECTED_ROM_ADDRESS_MASK_REPORT_ID:
			return 3;

		case I2C_ROM_BANK_WRITE_PROTECT_FLAG_REPORT_ID:
			return 2;

		case I2C_ROM_CONTENTS_REPORT_ID:
			return 1037;

		case I2C_RAM_TRANSACTION_REPORT_ID:
		case I2C_ROM_TRANSACTION_REPORT_ID:
			return 40;

		default:
			if (reportId & I2C_TRANSACTION_REPORT_FLAG)
				return I2C_TRANSACTION_REPORT_LENGTH_FROM(reportId);

			return -1;
	}
}

uint8_t *usbI2cGetOutputReportBufferFor(uint8_t reportId)
{
	if (reportId == I2C_ROM_CONTENTS_REPORT_ID || reportId == I2C_RAM_INITIALISATION_CONTENTS_REPORT_ID)
	{
		if (i2cMasterIsIdleFromIsr())
			return usbI2cOutputTransactionReportBuffer;
		else
			return (uint8_t *) 0;
	}

	if ((reportId & I2C_TRANSACTION_REPORT_FLAG) == 0)
	{
		usbI2cOutputReportBufferIndex ^= 1;
		usbI2cOutputReportBuffer = usbI2cOutputReportBuffers[usbI2cOutputReportBufferIndex];
		return usbI2cOutputReportBuffer;
	}

	uint8_t *bufferPtr = usbI2cOutputTransactionReportBufferPtr;
	usbI2cOutputTransactionReportBufferPtr += I2C_TRANSACTION_REPORT_LENGTH_FROM(reportId);
	if (usbI2cOutputTransactionReportBufferPtr > (usbI2cOutputTransactionReportBuffer + sizeof(usbI2cOutputTransactionReportBuffer)))
	{
		usbI2cOutputTransactionReportBufferPtr = usbI2cOutputTransactionReportBuffer;
		bufferPtr = usbI2cOutputTransactionReportBufferPtr;
	}

	return bufferPtr;
}

int8_t usbI2cOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	if (xQueueSendToBackFromISR(usbI2cReportsQueue, &context, NULL) != pdPASS)
	{
		usbSendAcknowledgementReportFromIsr(endpoint, *((uint8_t *) context), REPORT_NACK_QUEUE_FULL);
		return MSTACK_STALL;
	}

	uint8_t reportId = *((uint8_t *) context);
	if (reportId == I2C_ROM_CONTENTS_REPORT_ID || reportId == I2C_RAM_INITIALISATION_CONTENTS_REPORT_ID)
		i2cDisable();

	return 0;
}

uint8_t *usbI2cGetInputReportFor(uint8_t reportType, uint8_t reportId)
{
	uint8_t *report;
	if (reportId == I2C_ROM_CONTENTS_REPORT_ID || reportId == I2C_RAM_INITIALISATION_CONTENTS_REPORT_ID)
	{
		if (i2cMasterIsIdleFromIsr())
			report = usbI2cOutputTransactionReportBuffer;
		else
			return (uint8_t *) 0;
	}
	else
	{
		report = usbI2cInputReportBuffers[usbI2cInputReportBufferIndex];
		usbI2cInputReportBufferIndex ^= 1;
	}

	int16_t status = -1;
	switch (reportId)
	{
		case I2C_BUS_CONFIGURATION_REPORT_ID:
			status = usbI2cGetBusConfigurationReport(report);
			break;

		case I2C_MASTER_STORED_CONFIGURATION_REPORT_ID:
			status = usbI2cGetMasterStoredConfigurationReport(report);
			break;

		case I2C_MASTER_IMMEDIATE_CONFIGURATION_REPORT_ID:
			status = usbI2cGetMasterImmediateConfigurationReport(report);
			break;

		case I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID:
			status = usbI2cGetSlaveStoredConfigurationReport(report);
			break;

		case I2C_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID:
			status = usbI2cGetSlaveImmediateConfigurationReport(report);
			break;

		case I2C_SLAVE_REPORT_CONFIGURATION_REPORT_ID:
			status = usbI2cGetSlaveReportConfigurationReport(report);
			break;

		case I2C_PROTECTED_RAM_ADDRESS_MASK_REPORT_ID:
			status = usbI2cGetProtectedRamAddressMaskReport(report);
			break;

		case I2C_RAM_BANK_WRITE_PROTECT_FLAG_REPORT_ID:
			status = usbI2cGetRamBankWriteProtectFlagReport(report);
			break;

		case I2C_RAM_INITIALISATION_CONTENTS_REPORT_ID:
			status = usbI2cGetRamInitialisationContentsReport(report);
			break;

		case I2C_PROTECTED_ROM_ADDRESS_MASK_REPORT_ID:
			status = usbI2cGetProtectedRomAddressMaskReport(report);
			break;

		case I2C_ROM_BANK_WRITE_PROTECT_FLAG_REPORT_ID:
			status = usbI2cGetRomBankWriteProtectFlagReport(report);
			break;

		case I2C_ROM_CONTENTS_REPORT_ID:
			status = usbI2cGetRomContentsReport(report);
			break;

		default:
			break;
	}

	return status >= 0 ? report : (uint8_t *) 0;
}
