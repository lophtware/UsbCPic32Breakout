#include <xc.h>
#include <stdint.h>

#include "../FreeRtos.h"

#include "BenchTesting.h"

QueueHandle_t benchTestingCommands;

void benchTestingInitialise(void)
{
	xTaskCreate(&benchTestingTask, "TEST", RESERVE_STACK_USAGE_BYTES(552), NULL, configMAX_SYSCALL_INTERRUPT_PRIORITY, NULL);
	benchTestingCommands = xQueueCreate(4, sizeof(struct BenchTestingCommand));
}
