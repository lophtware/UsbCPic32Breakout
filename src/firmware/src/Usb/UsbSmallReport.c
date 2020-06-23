#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"

#include "Usb.h"

static void usbSmallReportsTask(void *args);
static bool usbSmallReportCanBeSentTo(uint8_t endpoint);

static QueueHandle_t usbSmallReportQueue;
static bool usbSmallReportsDisabled[NUM_ENDPOINT_NUMBERS];

void usbSmallReportInitialise(void)
{
	usbSmallReportQueue = xQueueCreate(16, sizeof(struct UsbSmallReport));
	xTaskCreate(&usbSmallReportsTask, "USB-SMRP", RESERVE_STACK_USAGE_BYTES(560), NULL, 3, NULL);
}

static void usbSmallReportsTask(void *args)
{
	static struct UsbSmallReport report;
	while (true)
	{
		xQueueReceive(usbSmallReportQueue, &report, portMAX_DELAY);
		static uint8_t endpoint;
		endpoint = report.as.fields.flags.endpoint;
		while (!usbSmallReportCanBeSentTo(endpoint))
			;;

		static uint32_t *buffer;
		buffer = (uint32_t *) usb_get_in_buffer(endpoint);
		buffer[0] = report.as.dwords[0];
		buffer[1] = report.as.dwords[1];
		usb_send_in_buffer(endpoint, report.as.fields.flags.count);
	}
}

static bool usbSmallReportCanBeSentTo(uint8_t endpoint)
{
	return !(usbSmallReportsDisabled[endpoint] || usbIsInEndpointBusy(endpoint));
}

bool usbSmallReportSend(struct UsbSmallReport report)
{
	return xQueueSend(usbSmallReportQueue, &report, portMAX_DELAY) == pdPASS;
}

bool usbSmallReportSendFromIsr(struct UsbSmallReport report, BaseType_t *wasHigherPriorityTaskWoken)
{
	return xQueueSendFromISR(usbSmallReportQueue, &report, wasHigherPriorityTaskWoken) == pdPASS;
}

void usbSmallReportEnableFor(uint8_t endpoint)
{
	if (endpoint < NUM_ENDPOINT_NUMBERS)
		usbSmallReportsDisabled[endpoint] = false;
}

void usbSmallReportDisableFor(uint8_t endpoint)
{
	if (endpoint < NUM_ENDPOINT_NUMBERS)
		usbSmallReportsDisabled[endpoint] = true;
}
