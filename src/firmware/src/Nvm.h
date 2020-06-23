#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_NVM_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_NVM_H
#include <stdint.h>
#include <stdbool.h>

uint8_t nvmErasePages2048(volatile const void *ptr, uint8_t numPages);
extern bool nvmWriteRow256(volatile const void *destPtr, const void *srcPtr);
extern bool nvmWriteDword(volatile const void *ptr, uint64_t dword);

#endif
