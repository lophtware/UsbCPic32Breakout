#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FREERTOS_FREERTOSCONFIG_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FREERTOS_FREERTOSCONFIG_H
#include <xc.h>

#define configUSE_PREEMPTION 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configTICK_RATE_HZ ((TickType_t) 500)
#define configCPU_CLOCK_HZ (24000000ul)
#define configPERIPHERAL_CLOCK_HZ (24000000ul)
#define configMAX_PRIORITIES (4)
#define configMINIMAL_STACK_SIZE (120)
#define configISR_STACK_SIZE (350)
#define configAPPLICATION_ALLOCATED_HEAP 1
#define configTOTAL_HEAP_SIZE ((size_t) 12 * 1024)
#define configMAX_TASK_NAME_LEN (8)
#define configUSE_TRACE_FACILITY 0
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1
#define configUSE_MUTEXES 1
#define configCHECK_FOR_STACK_OVERFLOW 3
#define configQUEUE_REGISTRY_SIZE 0
#define configUSE_QUEUE_SETS 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_MALLOC_FAILED_HOOK 1
#define configUSE_APPLICATION_TASK_TAG 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configGENERATE_RUN_TIME_STATS 0

#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

#define configUSE_TIMERS 0
#define configTIMER_TASK_PRIORITY (2)
#define configTIMER_QUEUE_LENGTH 4
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2)

#define INCLUDE_vTaskPrioritySet 0
#define INCLUDE_uxTaskPriorityGet 0
#define INCLUDE_vTaskDelete 0
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_eTaskGetState 1

#ifndef __LANGUAGE_ASSEMBLY
extern void faultAssertionFail(const char *file, unsigned long line);
#define configASSERT(x) if((x) == 0) faultAssertionFail(__FILE__, __LINE__)
#endif

#define configKERNEL_INTERRUPT_PRIORITY 1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 4

#define configCORE_TIMER_TICKS_PER_RTOS_TICK ((configCPU_CLOCK_HZ / 2) / configTICK_RATE_HZ)

#endif
