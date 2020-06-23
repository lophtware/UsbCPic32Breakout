#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_H
#include <stdint.h>
#include <stdbool.h>
#include "FreeRtos.h"

#define USB_MODULE_ID 0x03

extern void usbInitialise(QueueHandle_t flagsEventQueue, QueueHandle_t i2cEventQueue);
extern bool usbWasDedicatedChargingPortTestPerformed(void);
extern bool usbIsAttachedToDedicatedChargingPort(void);

extern uint8_t usbUsbI2cBankReadByte(uint16_t address);
extern bool usbUsbI2cBankWriteByte(uint16_t address, uint8_t value);

extern uint8_t usbI2cBankReadByte(uint16_t address);
extern bool usbI2cBankWriteByte(uint16_t address, uint8_t value);

#endif
