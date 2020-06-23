#include <xc.h>
#include <stdint.h>

#include "../FreeRtos.h"

#if (defined(configTICK_INTERRUPT_VECTOR) && configTICK_INTERRUPT_VECTOR == _CORE_TIMER_VECTOR)

void vApplicationSetupTickTimerInterrupt(void)
{
	IPC0bits.CTIP = configKERNEL_INTERRUPT_PRIORITY;
	IPC0bits.CTIS = 0;

	configCLEAR_TICK_TIMER_INTERRUPT();
	IEC0SET = _IEC0_CTIE_MASK;
}

#endif
