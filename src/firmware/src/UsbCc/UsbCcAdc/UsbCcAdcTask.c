#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../FreeRtos.h"
#include "../../EventQueueHeader.h"
#include "../../Adc.h"

#include "../UsbCc.h"

#include "UsbCcAdc.h"

#define USB_CC_DEBOUNCE_TIME_MS 150

static bool usbCcAdcOnCc1Sampled(struct AdcTransaction *transaction);
static uint16_t meanOfInterquartileRange(uint16_t *samples, uint16_t numberOfSamples);
static inline void insertionSort(uint16_t *samples, uint16_t numberOfSamples);
static bool usbCcAdcOnCc2Sampled(struct AdcTransaction *transaction);

void usbCcAdcTask(void *args)
{
	static uint16_t cc1;
	static uint16_t cc2;
	static uint16_t usbCcAdcSamples[64];
	static const struct AdcTransaction cc1Transaction =
	{
		.sampleBuffer = usbCcAdcSamples,
		.numberOfSamples = sizeof(usbCcAdcSamples) / sizeof(uint16_t),
		.onDone = &usbCcAdcOnCc1Sampled,
		.context = &cc1,
		.tadDivision = ADC_TAD_DIVISION_FASTEST,
		.tadBetweenSamples = ADC_TAD_BETWEEN_FASTEST,
		.channel = ADC_CHANNEL_AN9,
		.flags =
		{
			.bits =
			{
				.is12Bit = 0,
				.sampleFormat = ADC_SAMPLE_FORMAT_UINT
			}
		}
	};

	static const struct AdcTransaction cc2Transaction =
	{
		.sampleBuffer = usbCcAdcSamples,
		.numberOfSamples = sizeof(usbCcAdcSamples) / sizeof(uint16_t),
		.onDone = &usbCcAdcOnCc2Sampled,
		.context = &cc2,
		.tadDivision = ADC_TAD_DIVISION_FASTEST,
		.tadBetweenSamples = ADC_TAD_BETWEEN_FASTEST,
		.channel = ADC_CHANNEL_AN10,
		.flags =
		{
			.bits =
			{
				.is12Bit = 0,
				.sampleFormat = ADC_SAMPLE_FORMAT_UINT
			}
		}
	};

	vTaskDelay(pdMS_TO_TICKS(USB_CC_DEBOUNCE_TIME_MS));
	while (true)
	{
		while (!adcTransactionTryStart(&cc1Transaction))
			vTaskDelay(1);

		while (!adcTransactionTryStart(&cc2Transaction))
			vTaskDelay(1);

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		static struct UsbCcEvent event = { .header = EVENT_QUEUE_HEADER_INIT(USBCC_MODULE_ID, USBCC_EVENT_FLAGS_CHANGED) };
		usbCcAdcDecodeCcVoltagesIntoFlags(&event, cc1, cc2);
		xQueueSendToBack(usbCcAdcEvents, &event, pdMS_TO_TICKS(10));

		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

static bool usbCcAdcOnCc1Sampled(struct AdcTransaction *transaction)
{
	*((uint16_t *) transaction->context) = meanOfInterquartileRange(transaction->sampleBuffer, transaction->numberOfSamples);
	return false;
}

static uint16_t meanOfInterquartileRange(uint16_t *samples, uint16_t numberOfSamples)
{
	insertionSort(samples, numberOfSamples);

	uint32_t sum = 0;
	uint16_t quarterOffset = numberOfSamples / 4;
	for (uint16_t i = quarterOffset; i < numberOfSamples - quarterOffset; i++)
		sum += samples[i];
	
	return (sum + 1) / (numberOfSamples / 2);
}

static inline void insertionSort(uint16_t *samples, uint16_t numberOfSamples)
{
	for (uint16_t i = 1; i < numberOfSamples; i++)
	{
		for (uint16_t j = i; j > 0 && samples[j - 1] > samples[j]; j--)
		{
			uint16_t temp = samples[j];
			samples[j] = samples[j - 1];
			samples[j - 1] = temp;
		}
	}
}

static bool usbCcAdcOnCc2Sampled(struct AdcTransaction *transaction)
{
	*((uint16_t *) transaction->context) = meanOfInterquartileRange(transaction->sampleBuffer, transaction->numberOfSamples);
	xTaskNotifyGive(usbCcAdcTaskHandle);
	return false;
}
