#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FREERTOS_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FREERTOS_H
#include "FreeRtos/FreeRTOSConfig.h"
#include "FreeRtos/FreeRtos/include/FreeRTOS.h"
#include "FreeRtos/FreeRtos/include/task.h"
#include "FreeRtos/FreeRtos/include/queue.h"
#include "FreeRtos/FreeRtos/include/stack_macros.h"

#define STACK_DEPTH_CONTINGENCY_WORDS 4
#define HIGH_PRIORITY_ISR_STACK_USAGE_BYTES 0 /* If the DMA interrupts are used, this needs to be 160 */
#define RESERVE_STACK_USAGE_BYTES(x) ((((x) + HIGH_PRIORITY_ISR_STACK_USAGE_BYTES) / 4) + STACK_DEPTH_CONTINGENCY_WORDS)

#endif
