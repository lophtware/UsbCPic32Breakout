#include <xc.h>

#include "Syskey.h"

void syskeyUnlockThen(SyskeyUnlockCallback callback)
{
	if (!callback)
		return;

	while (SYSKEY == 0)
	{
		SYSKEY = 0;
		SYSKEY = 0xaa996655;
		SYSKEY = 0x556699aa;
	}

	callback();
	SYSKEY = 0;
}
