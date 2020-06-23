#include <xc.h>
#include <sys/asm.h>

#include "ISR_Support.h"

	.set nomips16
	.set noreorder

	.extern pinsChangedIsr
	.global _pinsChangedInterruptA
	.global _pinsChangedInterruptB
	.global _pinsChangedInterruptC

	.set noreorder
	.set noat

	.ent _pinsChangedInterruptA
_pinsChangedInterruptA:

	.aent _pinsChangedInterruptB
_pinsChangedInterruptB:

	.aent _pinsChangedInterruptC
_pinsChangedInterruptC:

	portSAVE_CONTEXT
	jal pinsChangedIsr
	nop
	portRESTORE_CONTEXT

	.end _pinsChangedInterruptA
