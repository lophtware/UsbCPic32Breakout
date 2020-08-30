#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_ADC_ADC_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_ADC_ADC_H
#include <stdint.h>
#include "../FreeRtos.h"
#include "../Adc.h"

extern TaskHandle_t adcTaskHandle;
extern QueueHandle_t adcTransactions;

extern void adcTask(void *args);
extern void adcOnTransactionDequeued(struct AdcTransaction *transaction);

#endif
