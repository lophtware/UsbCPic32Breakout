#include <xc.h>
#include <stdint.h>

#include "../../FreeRtos.h"

#include "Fusb303.h"

void fusb303I2cIsr(void)
{
	IFS2CLR = _IFS2_I2C1MIF_MASK;
	BaseType_t wasHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(fusb303TaskHandle, &wasHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(wasHigherPriorityTaskWoken);
}

void fusb303IntPinIsr(void)
{
	IEC0CLR = _IEC0_INT1IE_MASK;
	BaseType_t wasHigherPriorityTaskWoken = pdFALSE;
	fusb303ReadFlagsAndBroadcastFromIsr(&wasHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(wasHigherPriorityTaskWoken);
}
