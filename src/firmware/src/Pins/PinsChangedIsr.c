#include <xc.h>
#include <stdint.h>

#include "../FreeRtos.h"

#include "Pins.h"

static uint32_t pinsChangedTimestamp;
static TaskHandle_t pinsChangedTaskHandle;

void pinsChangedIsr(void)
{
	pinsChangedTimestamp = _CP0_GET_COUNT();

	IFS0CLR = (_IFS0_CNAIF_MASK | _IFS0_CNBIF_MASK | _IFS0_CNCIF_MASK);
	CNEN0ACLR = CONFIGURABLE_A_MASK;
	CNEN1ACLR = CONFIGURABLE_A_MASK;

	CNEN0BCLR = CONFIGURABLE_B_MASK;
	CNEN1BCLR = CONFIGURABLE_B_MASK;

	CNEN0CCLR = CONFIGURABLE_C_MASK;
	CNEN1CCLR = CONFIGURABLE_C_MASK;

	BaseType_t wasHigherPriorityTaskWoken = pdFALSE;
	if (pinsChangedTaskHandle)
		vTaskNotifyGiveFromISR(pinsChangedTaskHandle, &wasHigherPriorityTaskWoken);

	portEND_SWITCHING_ISR(wasHigherPriorityTaskWoken);
}

void pinsOnChangedNotify(TaskHandle_t pinsChanged)
{
	pinsChangedTaskHandle = pinsChanged;
}

uint32_t pinsGetLastOnChangedTimestamp(void)
{
	return pinsChangedTimestamp;
}

void pinsChangedReset(void)
{
	if (!pins)
		return;

	taskENTER_CRITICAL();
	CNEN0ASET = pins->bankStates[0].cnen0 & pins->bankStates[0].cnenIsContinuous;
	CNEN1ASET = pins->bankStates[0].cnen1 & pins->bankStates[0].cnenIsContinuous;

	CNEN0BSET = pins->bankStates[1].cnen0 & pins->bankStates[1].cnenIsContinuous;
	CNEN1BSET = pins->bankStates[1].cnen1 & pins->bankStates[1].cnenIsContinuous;

	CNEN0CSET = pins->bankStates[2].cnen0 & pins->bankStates[2].cnenIsContinuous;
	CNEN1CSET = pins->bankStates[2].cnen1 & pins->bankStates[2].cnenIsContinuous;
	taskEXIT_CRITICAL();
}

void pinsChangedResetNonContinuous(uint16_t a, uint16_t b, uint16_t c)
{
	if (!pins)
		return;

	taskENTER_CRITICAL();
	CNEN0ASET = pins->bankStates[0].cnen0 & a & (~pins->bankStates[0].cnenIsContinuous) & CONFIGURABLE_A_MASK;
	CNEN1ASET = pins->bankStates[0].cnen1 & a & (~pins->bankStates[0].cnenIsContinuous) & CONFIGURABLE_A_MASK;

	CNEN0BSET = pins->bankStates[1].cnen0 & b & (~pins->bankStates[1].cnenIsContinuous) & CONFIGURABLE_B_MASK;
	CNEN1BSET = pins->bankStates[1].cnen1 & b & (~pins->bankStates[1].cnenIsContinuous) & CONFIGURABLE_B_MASK;

	CNEN0CSET = pins->bankStates[2].cnen0 & c & (~pins->bankStates[2].cnenIsContinuous) & CONFIGURABLE_C_MASK;
	CNEN1CSET = pins->bankStates[2].cnen1 & c & (~pins->bankStates[2].cnenIsContinuous) & CONFIGURABLE_C_MASK;
	taskEXIT_CRITICAL();
}
