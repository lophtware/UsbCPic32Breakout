#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "Fault.h"
#include "Syskey.h"
#include "FreeRtos.h"
#include "Pins.h"

#define SAVE_RA_IN_FAULT() \
	__extension__({ \
		__asm__ volatile( \
			"move %0, $ra" \
			: "=d"(fault.ra)); \
	});

static void faultDoReset(void);
static void faultResetWithExceptionDetailWithoutRa(uint32_t reason);

__attribute__((persistent))
static struct
{
	uint32_t resetReason;
	uint32_t errorEpc;
	uint32_t epc;
	uint32_t cause;
	uint32_t desave;
	uint32_t ra;
	uint64_t debug;
} fault;

void faultUnrecoverableInitialisationError(void)
{
	__builtin_disable_interrupts();
	SAVE_RA_IN_FAULT();
	TRISASET = CONFIGURABLE_A_MASK;
	ANSELASET = CONFIGURABLE_A_MASK;
	TRISBSET = CONFIGURABLE_B_MASK;
	ANSELBSET = CONFIGURABLE_B_MASK;
	TRISCSET = CONFIGURABLE_C_MASK;
	ANSELCSET = CONFIGURABLE_C_MASK;
	while (true)
		_wait();
}

void _general_exception_handler(void)
{
	__builtin_disable_interrupts();
	RESET_STACK_POINTER_TO_TOP();
	faultResetWithExceptionDetail(RESET_REASON_GENERAL_EXCEPTION);
}

void faultAssertionFail(const char *file, unsigned long line)
{
	__builtin_disable_interrupts();
	SAVE_RA_IN_FAULT();
	_CP0_SET_DESAVE(((uint32_t) file[0] << 24) | ((uint32_t) file[1] << 16) | ((uint32_t) file[2] << 8) | file[3]);
	RESET_STACK_POINTER_TO_TOP();
	faultResetWithExceptionDetailWithoutRa(RESET_REASON_RTOS_ASSERTION_FAIL);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	__builtin_disable_interrupts();
	RESET_STACK_POINTER_TO_TOP();
	_CP0_SET_DESAVE(((uint32_t) pcTaskName[0] << 24) | ((uint32_t) pcTaskName[1] << 16) | ((uint32_t) pcTaskName[2] << 8) | pcTaskName[3]);
	faultResetWithExceptionDetail(RESET_REASON_RTOS_STACK_OVERFLOW);
}

void vApplicationMallocFailedHook(void)
{
	__builtin_disable_interrupts();
	RESET_STACK_POINTER_TO_TOP();
	faultResetWithExceptionDetail(RESET_REASON_RTOS_MALLOC);
}

void faultReset(uint32_t reason)
{
	fault.resetReason = reason;
	fault.errorEpc = 0;
	fault.epc = 0;
	fault.cause = 0;
	fault.desave = 0;
	SAVE_RA_IN_FAULT();
	syskeyUnlockThen(faultDoReset);
}

static void faultDoReset(void)
{
	RCON = 0;
	RSWRSTSET = _RSWRST_SWRST_MASK;
	while (RSWRST | 1)
		;;
}

void faultResetWithExceptionDetail(uint32_t reason)
{
	SAVE_RA_IN_FAULT();
	faultResetWithExceptionDetailWithoutRa(reason);
}

static void faultResetWithExceptionDetailWithoutRa(uint32_t reason)
{
	fault.resetReason = reason;
	fault.errorEpc = _CP0_GET_ERROREPC();
	fault.epc = _CP0_GET_EPC();
	fault.cause = (_CP0_GET_CAUSE() & 0x7c) >> 2;
	fault.desave = _CP0_GET_DESAVE();
	syskeyUnlockThen(faultDoReset);
}

uint32_t faultGetResetReason(void)
{
	if (RCON && !(RCON & _RCON_SWR_MASK))
		fault.resetReason = RCON;

	RCON = 0;
	return fault.resetReason;
}

uint32_t faultGetExceptionDetailErrorEpc(void)
{
	return fault.errorEpc;
}

uint32_t faultGetExceptionDetailEpc(void)
{
	return fault.epc;
}

uint32_t faultGetExceptionDetailCause(void)
{
	return fault.cause;
}

uint32_t faultGetExceptionDetailDesave(void)
{
	return fault.desave;
}

uint32_t faultGetExceptionDetailRa(void)
{
	return fault.ra;
}

void faultSetDebugDword(uint64_t dword)
{
	fault.debug = dword;
}

uint64_t faultGetDebugDword(void)
{
	return fault.debug;
}
