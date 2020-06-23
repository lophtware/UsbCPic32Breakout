#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../Spi.h"
#include "../../FreeRtos.h"

#include "UsbSpiInterface.h"

#define SPI_TRANSACTION_REPORT_HEADER_LENGTH 5
#define IS_LAST_REPORT_FLAG 0x02
#define IS_TIMEOUT_FLAG 0x01
#define MAXIMUM_IN_BUFFER_SPACE (sizeof(usbSpiTransactionReportBuffer) - SPI_TRANSACTION_REPORT_HEADER_LENGTH)

struct SpiMasterTransactionContext
{
	uint8_t *chunkReportPtr;
	uint16_t chunkBytesTransferred;
	uint16_t inLength;
	TaskHandle_t spiPollTaskHandle;
	bool hasLastReportBeenSent;
};

static void usbSpiTransactionOnChunkReceived(const struct SpiMasterTransaction *transaction, struct SpiMasterOnChunkReceivedEventArgs *args);
static int8_t usbSpiNotifyPollTaskOfCapacity(uint8_t ep, bool dataOk, void *context);
static bool usbSpiTransactionOnRepeat(struct SpiMasterTransaction *transaction);
static bool usbSpiTransactionOnDone(const struct SpiMasterTransaction *transaction);

void usbSpiOnTransactionReportReceived(const uint8_t *report)
{
	static struct SpiMasterTransactionContext context;
	context.chunkReportPtr = usbSpiTransactionReportBuffer;
	context.chunkBytesTransferred = 0;
	context.inLength = (((uint16_t) report[3]) << 8) | report[2];
	context.spiPollTaskHandle = (TaskHandle_t) 0;
	context.hasLastReportBeenSent = false;

	struct SpiMasterTransaction transaction =
	{
		.flags.bits =
		{
			.slaveAddress = report[4] & 0x07
		},

		.outLength = ((uint16_t) (report[0] & ~SPI_TRANSACTION_REPORT_FLAG)) * 8 + ((report[1] >> 5) & 0x07),
		.out = &report[SPI_TRANSACTION_REPORT_HEADER_LENGTH],

		.inLength = (context.inLength > MAXIMUM_IN_BUFFER_SPACE) ? MAXIMUM_IN_BUFFER_SPACE : context.inLength,
		.inChunkLength = EP_SPI_IN_LEN - SPI_TRANSACTION_REPORT_HEADER_LENGTH,
		.in = &context.chunkReportPtr[SPI_TRANSACTION_REPORT_HEADER_LENGTH],
		.onChunkReceived = &usbSpiTransactionOnChunkReceived,

		.onRepeat = (context.inLength > MAXIMUM_IN_BUFFER_SPACE) ? &usbSpiTransactionOnRepeat : (SpiMasterTransactionOnRepeat) 0,

		.onDone = &usbSpiTransactionOnDone,
		.context = &context
	};

	if (spiMasterSend(&transaction))
		usbSmallReportDisableFor(SPI_EP_ID);
	else
		usbSendAcknowledgementReport(SPI_EP_ID, report[0], REPORT_NACK_QUEUE_FULL);
}

static void usbSpiTransactionOnChunkReceived(const struct SpiMasterTransaction *transaction, struct SpiMasterOnChunkReceivedEventArgs *args)
{
	if (usbIsInEndpointBusy(SPI_EP_ID))
	{
		args->consumed = 0;
		return;
	}

	if (args->length > 1023)
		args->length = 1023;

	struct SpiMasterTransactionContext *context = (struct SpiMasterTransactionContext *) transaction->context;
	context->spiPollTaskHandle = args->pollTaskHandle;

	uint8_t *report = context->chunkReportPtr;
	uint16_t remainingBytes = context->inLength - context->chunkBytesTransferred - args->length;
	bool isLastReport = transaction->statusFlags.bits.timeout || (transaction->inLength >= transaction->outLength && remainingBytes == 0);
	uint8_t lengthIn8ByteBlocks = (uint8_t) ((args->length >> 3) & 0x7f);
	report[0] = SPI_TRANSACTION_REPORT_FLAG | lengthIn8ByteBlocks;
	report[1] =
		((args->length & 0x07) << 5) |
		(isLastReport ? IS_LAST_REPORT_FLAG : 0x00) |
		(transaction->statusFlags.bits.timeout ? IS_TIMEOUT_FLAG : 0x00);

	report[2] = (uint8_t) ((remainingBytes >> 0) & 0xff);
	report[3] = (uint8_t) ((remainingBytes >> 8) & 0xff);
	report[4] = transaction->flags.bits.slaveAddress & 0x07;

	if (!usbStartSendEpDataStage(
		SPI_EP_ID,
		report,
		SPI_TRANSACTION_REPORT_HEADER_LENGTH + ((size_t) lengthIn8ByteBlocks << 3) + 7,
		&usbSpiNotifyPollTaskOfCapacity,
		context))
	{
		args->pollIntervalTicks = 1;
		args->consumed = 0;
		return;
	}

	args->consumed = args->length;
	args->pollIntervalTicks = (args->consumed >> 7) + 1;
	context->chunkReportPtr += args->consumed;
	context->chunkBytesTransferred += args->consumed;
	context->hasLastReportBeenSent = isLastReport;
}

static int8_t usbSpiNotifyPollTaskOfCapacity(uint8_t ep, bool dataOk, void *context)
{
	TaskHandle_t task = ((struct SpiMasterTransactionContext *) context)->spiPollTaskHandle;
	vTaskNotifyGiveFromISR(task, NULL);
	return 0;
}

static bool usbSpiTransactionOnRepeat(struct SpiMasterTransaction *transaction)
{
	struct SpiMasterTransactionContext *context = (struct SpiMasterTransactionContext *) transaction->context;
	if (context->chunkBytesTransferred >= context->inLength || context->hasLastReportBeenSent)
		return false;

	context->chunkReportPtr = usbSpiTransactionReportBuffer;

	transaction->inLength = context->inLength - context->chunkBytesTransferred;
	if (transaction->inLength > MAXIMUM_IN_BUFFER_SPACE)
		transaction->inLength = MAXIMUM_IN_BUFFER_SPACE;

	transaction->outLength = 0;

	while (usbIsInEndpointBusy(SPI_EP_ID))
		;;

	return true;
}

static bool usbSpiTransactionOnDone(const struct SpiMasterTransaction *transaction)
{
	struct SpiMasterTransactionContext *context = (struct SpiMasterTransactionContext *) transaction->context;
	if (context->hasLastReportBeenSent)
	{
		usbSmallReportEnableFor(SPI_EP_ID);
		return true;
	}

	while (usbIsInEndpointBusy(SPI_EP_ID))
		;;

	uint8_t *buffer = usb_get_in_buffer(SPI_EP_ID);
	buffer[0] = SPI_TRANSACTION_REPORT_FLAG;
	buffer[1] =
		IS_LAST_REPORT_FLAG |
		(transaction->statusFlags.bits.timeout ? IS_TIMEOUT_FLAG : 0x00);

	uint16_t bytesRemaining = context->inLength - context->chunkBytesTransferred;
	buffer[2] = (uint8_t) ((bytesRemaining >> 0) & 0xff);
	buffer[3] = (uint8_t) ((bytesRemaining >> 8) & 0xff);
	buffer[4] = transaction->flags.bits.slaveAddress;

	usb_send_in_buffer(SPI_EP_ID, SPI_TRANSACTION_REPORT_HEADER_LENGTH + 7);
	usbSmallReportEnableFor(SPI_EP_ID);

	return true;
}
