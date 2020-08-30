#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_FUSB303_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_FUSB303_H
#include <stdint.h>
#include <stdbool.h>
#include "../FreeRtos.h"

extern void fusb303Initialise(QueueHandle_t eventQueue);
extern bool fusb303IsInitialised(void);
extern uint16_t fusb303GetCurrentLimitMilliamps(void);

#endif
