#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_USBCCADC_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_USBCCADC_H
#include <stdint.h>
#include <stdbool.h>
#include "../FreeRtos.h"

extern void usbCcAdcInitialise(QueueHandle_t eventQueue);
extern bool usbCcAdcIsInitialised(void);
extern uint16_t usbCcAdcGetCurrentLimitMilliamps(void);

#endif
