#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../I2c.h"
#include "../../FreeRtos.h"

#include "UsbI2cInterface.h"

#define I2C_TRANSACTION_REPORT_HEADER_LENGTH 5

#define NO_CHUNK_SENT ((void *) 0)
#define AT_LEAST_ONE_CHUNK_SENT ((void *) 1)

static uint8_t *usbI2cTransactionGetInBuffer(void);
static void usbI2cTransactionOnChunkReceived(const struct I2cMasterTransaction *transaction, struct I2cMasterOnChunkReceivedEventArgs *args);
static bool usbI2cTransactionOnDone(const struct I2cMasterTransaction *transaction);

void usbI2cOnTransactionReportReceived(const uint8_t *report)
{
	struct I2cMasterTransaction transaction =
	{
		.deviceAddress = report[4],
		.flags.bits = { .isInBufferDelegate = 1 },

		.outLength = ((uint16_t) (report[0] & ~I2C_TRANSACTION_REPORT_FLAG)) * 8 + ((report[1] >> 5) & 0x07),
		.out = &report[I2C_TRANSACTION_REPORT_HEADER_LENGTH],

		.inLength = (((uint16_t) report[3]) << 8) | report[2],
		.inChunkLength = EP_I2C_IN_LEN - I2C_TRANSACTION_REPORT_HEADER_LENGTH,
		.in = { .asDelegate = &usbI2cTransactionGetInBuffer },
		.onChunkReceived = &usbI2cTransactionOnChunkReceived,

		.onDone = &usbI2cTransactionOnDone,
		.context = NO_CHUNK_SENT
	};

	if (!i2cMasterSend(&transaction))
		usbSendAcknowledgementReport(I2C_EP_ID, report[0], REPORT_NACK_QUEUE_FULL);
}

static uint8_t *usbI2cTransactionGetInBuffer(void)
{
	while (usbIsInEndpointBusy(I2C_EP_ID))
	{
		vTaskDelay(1);
	}

	usbSmallReportDisableFor(I2C_EP_ID);
	return usb_get_in_buffer(I2C_EP_ID) + I2C_TRANSACTION_REPORT_HEADER_LENGTH;
}

static void usbI2cTransactionOnChunkReceived(const struct I2cMasterTransaction *transaction, struct I2cMasterOnChunkReceivedEventArgs *args)
{
	uint8_t *buffer = usb_get_in_buffer(I2C_EP_ID);
	buffer[0] = I2C_TRANSACTION_REPORT_FLAG | ((args->length >> 3) & 0x7f);
	buffer[1] =
		((args->length << 5) & 0xe0) |
		(transaction->statusFlags.bits.writeDone ? 0x08 : 0x00) |
		(transaction->statusFlags.bits.collision ? 0x04 : 0x00) |
		(transaction->statusFlags.bits.dataNack ? 0x02 : 0x00) |
		(transaction->statusFlags.bits.addressNack ? 0x01 : 0x00) |
		(transaction->statusFlags.bits.timeout ? 0x03 : 0x00);

	buffer[2] = (uint8_t) ((transaction->inLength >> 0) & 0xff);
	buffer[3] = (uint8_t) ((transaction->inLength >> 8) & 0xff);
	buffer[4] = transaction->deviceAddress;

	usb_send_in_buffer(I2C_EP_ID, (args->length & 0xf8) + I2C_TRANSACTION_REPORT_HEADER_LENGTH + 7);
	((struct I2cMasterTransaction *) transaction)->context = AT_LEAST_ONE_CHUNK_SENT;
}

static bool usbI2cTransactionOnDone(const struct I2cMasterTransaction *transaction)
{
	if (transaction->context != NO_CHUNK_SENT)
		return true;

	uint8_t *buffer = usb_get_in_buffer(I2C_EP_ID);
	buffer[0] = I2C_TRANSACTION_REPORT_FLAG;
	buffer[1] =
		(transaction->statusFlags.bits.writeDone ? 0x08 : 0x00) |
		(transaction->statusFlags.bits.collision ? 0x04 : 0x00) |
		(transaction->statusFlags.bits.dataNack ? 0x02 : 0x00) |
		(transaction->statusFlags.bits.addressNack ? 0x01 : 0x00) |
		(transaction->statusFlags.bits.timeout ? 0x03 : 0x00);

	buffer[2] = (uint8_t) ((transaction->outLength >> 0) & 0xff);
	buffer[3] = (uint8_t) ((transaction->outLength >> 8) & 0xff);
	buffer[4] = transaction->deviceAddress;

	usb_send_in_buffer(I2C_EP_ID, I2C_TRANSACTION_REPORT_HEADER_LENGTH + 7);
	usbSmallReportEnableFor(I2C_EP_ID);
	return true;
}
