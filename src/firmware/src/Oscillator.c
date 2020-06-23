#include <xc.h>
#include <stdint.h>

#include "Syskey.h"
#include "Oscillator.h"

#define OSCCON_PLL 1
#define SPLLCON_ODIV_BY_4 (2 << _SPLLCON_PLLODIV_POSITION)
#define SPLLCON_MULT_BY_12 (5 << _SPLLCON_PLLMULT_POSITION)

static void switchToUsbCapableSelfTunedOscillator(void);

void oscillatorInitialise(void)
{
	syskeyUnlockThen(switchToUsbCapableSelfTunedOscillator);
	while (!CLKSTATbits.SPLLRDY || !CLKSTATbits.SPDIVRDY)
		;;
}

static void switchToUsbCapableSelfTunedOscillator(void)
{
	OSCTUN = _OSCTUN_SRC_MASK | _OSCTUN_ON_MASK;
	SPLLCON = SPLLCON_MULT_BY_12 | SPLLCON_ODIV_BY_4 | _SPLLCON_PLLICLK_MASK;
	OSCCONbits.NOSC = OSCCON_PLL;
	OSCCONSET = _OSCCON_OSWEN_LENGTH;
}
