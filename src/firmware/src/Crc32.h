#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_CRC32_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_CRC32_H
#include <stdint.h>

extern void crc32CalculateTable(uint32_t *table);
extern uint32_t crc32Calculate(const uint32_t *table, volatile const void *start, uint32_t length);

#endif
