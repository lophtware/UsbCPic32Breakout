#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/kmem.h>

#include "Dma.h"
#include "Nvm.h"

#define NVMCON_NVMOP_PAGE_ERASE_MASK 0x04
#define NVMCON_NVMOP_ROW_MASK 0x03
#define NVMCON_NVMOP_DWORD_MASK 0x02

static void nvmPerformOperation(void);

uint8_t nvmErasePages2048(volatile const void *ptr, uint8_t numPages)
{
	NVMADDR = KVA_TO_PA(ptr);
	if (NVMADDR & 2047)
		return 0;

	for (uint8_t i = 0; i < numPages; i++)
	{
		NVMADDR = KVA_TO_PA(ptr);
		NVMCONCLR = _NVMCON_WREN_MASK;
		NVMCON = NVMCON_NVMOP_PAGE_ERASE_MASK;
		NVMCONSET = _NVMCON_WREN_MASK;

		nvmPerformOperation();
		if (NVMCON & (_NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK))
			return i;

		ptr = ((volatile const uint8_t *) ptr) + 2048;
	}

	return numPages;
}

static void nvmPerformOperation(void)
{
	unsigned int status = __builtin_disable_interrupts();
	dmaSuspend();
	NVMKEY = 0xaa996655;
	NVMKEY = 0x556699aa;
	NVMCONSET = _NVMCON_WR_MASK;
	while (NVMCON & _NVMCON_WR_MASK)
		;;

	NVMCONCLR = _NVMCON_WREN_MASK;
	NVMKEY = 0;
	dmaResume();
	_mtc0(12, 0, status);
}

bool nvmWriteRow256(volatile const void *destPtr, const void *srcPtr)
{
	NVMSRCADDR = KVA_TO_PA(srcPtr);
	NVMADDR = KVA_TO_PA(destPtr);
	if (NVMADDR & 255)
		return false;

	NVMCONCLR = _NVMCON_WREN_MASK;
	NVMCON = NVMCON_NVMOP_ROW_MASK;
	NVMCONSET = _NVMCON_WREN_MASK;

	nvmPerformOperation();
	if (NVMCON & (_NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK))
		return false;

	return memcmp(srcPtr, (const void *) destPtr, 256) == 0 ? true : false;
}

bool nvmWriteDword(volatile const void *ptr, uint64_t dword)
{
	if (*((volatile const uint64_t *) ptr) == dword)
		return true;

	if ((*((volatile const uint64_t *) ptr) & dword) != dword)
		return false;

	NVMADDR = KVA_TO_PA(ptr);
	if (NVMADDR & 7)
		return false;

	volatile const uint32_t *destPtr = (volatile const uint32_t *) ptr;
	NVMDATA0 = ((uint32_t) (dword >>  0)) & *(destPtr + 0);
	NVMDATA1 = ((uint32_t) (dword >> 32)) & *(destPtr + 1);
	NVMCONCLR = _NVMCON_WREN_MASK;
	NVMCON = NVMCON_NVMOP_DWORD_MASK;
	NVMCONSET = _NVMCON_WREN_MASK;
	nvmPerformOperation();

	return *((volatile const uint64_t *) ptr) == dword;
}
