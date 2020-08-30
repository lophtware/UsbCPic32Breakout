#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_USBCCADC_USBCCADC_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_USBCCADC_USBCCADC_H
#include <stdint.h>
#include <stdbool.h>
#include "../../FreeRtos.h"
#include "../UsbCc.h"
#include "../UsbCcAdc.h"

extern QueueHandle_t usbCcAdcEvents;
extern TaskHandle_t usbCcAdcTaskHandle;
extern uint16_t usbCcAdcCurrentLimitMilliamps;

extern void usbCcAdcTask(void *args);
extern void usbCcAdcDecodeCcVoltagesIntoFlags(struct UsbCcEvent *event, uint16_t cc1, uint16_t cc2);

#endif
