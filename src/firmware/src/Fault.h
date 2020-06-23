#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FAULT_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FAULT_H

#define RESET_REASON(x) (0x00008000u | (x))
#define RESET_REASON_IS_SYSTEM(x) (((x) & 0x00008000u) == 0)
#define RESET_REASON_IS_FIRMWARE(x) (((x) & 0x00008000u) != 0)
#define RESET_REASON_HAS_FIRMWARE_FLAG(x, f) (RESET_REASON_IS_FIRMWARE(x) && ((x) & (f)))

#define RESET_REASON_FIRMWARE_FLAG_SAFEMODE 0x80000000u

#define RESET_REASON_NEEDS_SAFEMODE(x) RESET_REASON_HAS_FIRMWARE_FLAG((x), RESET_REASON_FIRMWARE_FLAG_SAFEMODE)

#define RESET_REASON_GENERAL_EXCEPTION RESET_REASON(0x0001)

#define RESET_REASON_RTOS(x) RESET_REASON(0x0040 | (x))
#define RESET_REASON_RTOS_ASSERTION_FAIL RESET_REASON_RTOS(1)
#define RESET_REASON_RTOS_STACK_OVERFLOW RESET_REASON_RTOS(2)
#define RESET_REASON_RTOS_MALLOC RESET_REASON_RTOS(3)

#define RESET_REASON_CONFIGURATION(x) RESET_REASON(0x0080 | (x))
#define RESET_REASON_CONFIGURATION_UPDATED RESET_REASON_CONFIGURATION(0)
#define RESET_REASON_INVALID_CONFIGURATION(x) (RESET_REASON_CONFIGURATION(x) | RESET_REASON_FIRMWARE_FLAG_SAFEMODE)

#define RESET_REASON_FUSB303(x) RESET_REASON(0x00c0 | (x))
#define RESET_REASON_FUSB303_QUEUE_FULL RESET_REASON_FUSB303(1)

#define RESET_REASON_I2C(x) RESET_REASON(0x00100 | (x))
#define RESET_REASON_I2C_NVM_UPDATED RESET_REASON_I2C(0)

#define RESET_STACK_POINTER_TO_TOP() \
	__extension__({ \
		__asm__ volatile( \
			"move $sp, %0\n\t" \
			: \
			: "d"(0x80007ff0) \
			: "sp" /* clobbers */); \
	})

extern void faultUnrecoverableInitialisationError(void);
extern void faultReset(uint32_t reason);
extern void faultResetWithExceptionDetail(uint32_t reason);
extern uint32_t faultGetResetReason(void);
extern uint32_t faultGetExceptionDetailErrorEpc(void);
extern uint32_t faultGetExceptionDetailEpc(void);
extern uint32_t faultGetExceptionDetailCause(void);
extern uint32_t faultGetExceptionDetailDesave(void);
extern uint32_t faultGetExceptionDetailRa(void);
extern void faultSetDebugDword(uint64_t dword);
extern uint64_t faultGetDebugDword(void);

#endif
