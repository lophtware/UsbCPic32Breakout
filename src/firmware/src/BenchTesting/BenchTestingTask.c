#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"
#include "../Pins.h"

#include "BenchTesting.h"

void benchTestingTask(void *args)
{
	static struct BenchTestingCommand command;
	while (true)
	{
		xQueueReceive(benchTestingCommands, &command, portMAX_DELAY);
		if (command.as.fields.type == COMMAND_TOGGLE_ALL_PINS)
		{
			benchTestingTaskToggleAllPinsForSeconds(((uint16_t) command.as.fields.args[1] << 8) | command.as.fields.args[0]);
		}
		else if (command.as.fields.type == COMMAND_TOGGLE_ALL_PINS_WITH_PWM)
		{
			benchTestingTaskToggleAllPinsWithPwmForSeconds(
				((uint16_t) command.as.fields.args[1] << 8) | command.as.fields.args[0],
				command.as.fields.args[2]);
		}
	}
}

void benchTestingSendRawCommand(uint32_t command)
{
	xQueueSendToBack(benchTestingCommands, &command, portMAX_DELAY);
}
