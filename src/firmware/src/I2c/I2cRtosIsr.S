#include <xc.h>
#include <sys/asm.h>

#include "ISR_Support.h"

	.set nomips16
	.set noreorder

	.extern xISRStackTop

	.extern i2cMasterIsr
	.global _i2c2MasterInterrupt

	.set noreorder
	.set noat
	.ent _i2c2MasterInterrupt

_i2c2MasterInterrupt:
	portSAVE_CONTEXT
	jal i2cMasterIsr
	nop
	portRESTORE_CONTEXT

	.end _i2c2MasterInterrupt


	.extern i2cSlaveIsr
	.global _i2c2SlaveInterrupt

	.set noreorder
	.set noat
	.ent _i2c2SlaveInterrupt

_i2c2SlaveInterrupt:
	portSAVE_CONTEXT
	jal i2cSlaveIsr
	nop
	portRESTORE_CONTEXT

	.end _i2c2SlaveInterrupt


	.extern i2cCollisionIsr
	.global _i2c2CollisionInterrupt

	.set noreorder
	.set noat
	.ent _i2c2CollisionInterrupt

_i2c2CollisionInterrupt:
	portSAVE_CONTEXT
	jal i2cCollisionIsr
	nop
	portRESTORE_CONTEXT

	.end _i2c2CollisionInterrupt
