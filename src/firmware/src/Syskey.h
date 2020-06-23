#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_SYSKEY_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_SYSKEY_H

typedef void (*SyskeyUnlockCallback)(void);

extern void syskeyUnlockThen(SyskeyUnlockCallback callback);

#endif
