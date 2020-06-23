#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../FreeRtos.h"
#include "../../I2c.h"

#include "UsbI2cInterface.h"

QueueHandle_t usbI2cEventQueue;
QueueHandle_t usbI2cReportsQueue;

void usbI2cReportsAndEventsTask(void *args)
{
	static QueueSetMemberHandle_t queue;
	while (true)
	{
		queue = xQueueSelectFromSet(usbI2cQueueSet, portMAX_DELAY);
		if (queue == usbI2cReportsQueue)
		{
			static const uint8_t *report;
			xQueueReceive(queue, &report, 0);
			if (!report)
				continue;

			switch (report[0])
			{
				case I2C_BUS_CONFIGURATION_REPORT_ID:
					usbI2cOnBusConfigurationReportReceived(report);
					break;

				case I2C_MASTER_STORED_CONFIGURATION_REPORT_ID:
					usbI2cOnMasterConfigurationReportReceived(report);
					break;

				case I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID:
					usbI2cOnSlaveConfigurationReportReceived(report);
					break;

				case I2C_SLAVE_REPORT_CONFIGURATION_REPORT_ID:
					usbI2cOnSlaveReportConfigurationReportReceived(report);
					break;

				case I2C_PROTECTED_RAM_ADDRESS_MASK_REPORT_ID:
					usbI2cOnProtectedRamAddressMaskReportReceived(report);
					break;

				case I2C_RAM_BANK_WRITE_PROTECT_FLAG_REPORT_ID:
					usbI2cOnRamBankWriteProtectFlagReportReceived(report);
					break;

				case I2C_RAM_INITIALISATION_CONTENTS_REPORT_ID:
					usbI2cOnRamInitialisationContentsReportReceived(report);
					break;

				case I2C_PROTECTED_ROM_ADDRESS_MASK_REPORT_ID:
					usbI2cOnProtectedRomAddressMaskReportReceived(report);
					break;

				case I2C_ROM_BANK_WRITE_PROTECT_FLAG_REPORT_ID:
					usbI2cOnRomBankWriteProtectFlagReportReceived(report);
					break;

				case I2C_ROM_CONTENTS_REPORT_ID:
					usbI2cOnRomContentsReportReceived(report);
					break;

				case I2C_RAM_TRANSACTION_REPORT_ID:
				case I2C_ROM_TRANSACTION_REPORT_ID:
					usbI2cOnRomAndRamTransactionReportReceived(report);
					break;

				default:
					if (report[0] & I2C_TRANSACTION_REPORT_FLAG)
						usbI2cOnTransactionReportReceived(report);
					else
						usbSendAcknowledgementReport(I2C_EP_ID, report[0], REPORT_NACK_UNKNOWN_ID);

					break;
			}
		}
		else
		{
			static struct I2cEvent event;
			xQueueReceive(usbI2cEventQueue, &event, 0);
			if (event.header.as.raw == EVENT_QUEUE_HEADER_WORD_FOR(I2C_MODULE_ID, I2C_EVENT_SLAVE_TRANSACTION_DONE))
			{
				static struct UsbSmallReport eventReport;
				eventReport.as.fields.flags.endpoint = I2C_EP_ID;
				eventReport.as.fields.flags.count = 7;
				eventReport.as.fields.payload[0] = I2C_SLAVE_TRANSACTION_DONE_REPORT_ID;
				eventReport.as.fields.payload[1] = event.as.slaveTransactionDone.flags.bits.isWrite ? 0x80 : 0x00;
				eventReport.as.fields.payload[2] = event.as.slaveTransactionDone.deviceAddress;
				eventReport.as.fields.payload[3] = (uint8_t) (event.as.slaveTransactionDone.registerAddress >> 0) & 0xff;
				eventReport.as.fields.payload[4] = (uint8_t) (event.as.slaveTransactionDone.registerAddress >> 8) & 0xff;
				eventReport.as.fields.payload[5] = (uint8_t) (event.as.slaveTransactionDone.registerCount >> 0) & 0xff;
				eventReport.as.fields.payload[6] = (uint8_t) (event.as.slaveTransactionDone.registerCount >> 8) & 0xff;
				while (!usbSmallReportSend(eventReport))
				{
					vTaskDelay(1);
				}
			}
		}
		
	}
}
