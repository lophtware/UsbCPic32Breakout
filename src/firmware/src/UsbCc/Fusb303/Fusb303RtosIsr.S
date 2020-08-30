#include <xc.h>
#include <sys/asm.h>

#include "ISR_Support.h"

	.set nomips16
	.set noreorder

	.extern xISRStackTop

	.extern fusb303I2cIsr
	.global _i2c1MasterInterrupt

	.set noreorder
	.set noat
	.ent _i2c1MasterInterrupt

_i2c1MasterInterrupt:
	portSAVE_CONTEXT
	jal fusb303I2cIsr
	nop
	portRESTORE_CONTEXT

	.end _i2c1MasterInterrupt


	.extern fusb303IntPinIsr
	.global _int1PinInterrupt

	.set noreorder
	.set noat
	.ent _int1PinInterrupt

_int1PinInterrupt:
	portSAVE_CONTEXT
	jal fusb303IntPinIsr
	nop
	portRESTORE_CONTEXT

	.end _int1PinInterrupt
