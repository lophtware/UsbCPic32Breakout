#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_ADC_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_ADC_H
#include <stdint.h>
#include <stdbool.h>

#define ADC_SAMPLE_FORMAT_FRAC 3
#define ADC_SAMPLE_FORMAT_UFRAC 2
#define ADC_SAMPLE_FORMAT_INT 1
#define ADC_SAMPLE_FORMAT_UINT 0

#define ADC_CHANNEL_AVDD 0x1e
#define ADC_CHANNEL_AVSS 0x1d
#define ADC_CHANNEL_VBANDGAP 0x1c
#define ADC_CHANNEL_VDDCORE 0x1b

#define ADC_CHANNEL_AN11 0x0b
#define ADC_CHANNEL_AN10 0x0a
#define ADC_CHANNEL_AN9 0x09
#define ADC_CHANNEL_AN8 0x08
#define ADC_CHANNEL_AN7 0x07
#define ADC_CHANNEL_AN6 0x06
#define ADC_CHANNEL_AN5 0x05
#define ADC_CHANNEL_AN4 0x04
#define ADC_CHANNEL_AN3 0x03
#define ADC_CHANNEL_AN2 0x02
#define ADC_CHANNEL_AN1 0x01
#define ADC_CHANNEL_AN0 0x00

#define ADC_TAD_DIVISION_FASTEST 4
#define ADC_TAD_DIVISION_SLOWEST 255
#define ADC_TAD_BETWEEN_FASTEST 1
#define ADC_TAD_BETWEEN_SLOWEST 31

#define ADC_VOLTAGE_10BIT(x) ((uint16_t) ((x) * 1024 / 3.3 + 0.5))
#define ADC_VOLTAGE_12BIT(x) ((uint16_t) ((x) * 4096 / 3.3 + 0.5))

struct AdcTransaction;
typedef bool (*AdcCallback)(struct AdcTransaction *transaction);

struct AdcTransaction
{
	uint16_t *sampleBuffer;
	uint16_t numberOfSamples;

	AdcCallback onDone;
	void *context;

	uint8_t tadDivision;
	uint8_t tadBetweenSamples;
	uint8_t channel;
	union
	{
		uint8_t all;
		struct
		{
			unsigned int is12Bit : 1;
			unsigned int sampleFormat : 3;
			unsigned int : 4;
		} bits;
	} flags;
};

extern void adcInitialise(void);
extern bool adcTransactionTryStart(const struct AdcTransaction *transaction);

#endif
