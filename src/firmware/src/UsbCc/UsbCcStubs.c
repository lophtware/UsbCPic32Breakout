#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"

#include "UsbCc.h"

#if BOARD_VARIANT_HAS_FUSB303
#include "Fusb303.h"
#define PREFIXED_CALL(fn) fusb303##fn
#else
#include "UsbCcAdc.h"
#define PREFIXED_CALL(fn) usbCcAdc##fn
#endif

void usbCcInitialise(QueueHandle_t eventQueue)
{
	PREFIXED_CALL(Initialise(eventQueue));
}

bool usbCcIsInitialised(void)
{
	return PREFIXED_CALL(IsInitialised());
}

uint16_t usbCcGetCurrentLimitMilliamps(void)
{
	return PREFIXED_CALL(GetCurrentLimitMilliamps());
}
