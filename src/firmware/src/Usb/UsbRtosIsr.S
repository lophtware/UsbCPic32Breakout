#include <xc.h>
#include <sys/asm.h>

#include "ISR_Support.h"

	.set nomips16
	.set noreorder

	.extern usb_service
	.extern xISRStackTop
	.extern usbInterruptHasBeenCalled
	.global _USB1Interrupt

	.set noreorder
	.set noat
	.ent _USB1Interrupt

_USB1Interrupt:
	portSAVE_CONTEXT
	la t1, (usbInterruptHasBeenCalled)
	ori t0, zero, 1
	jal usb_service
	sw t0, (t1)
	portRESTORE_CONTEXT

	.end _USB1Interrupt
