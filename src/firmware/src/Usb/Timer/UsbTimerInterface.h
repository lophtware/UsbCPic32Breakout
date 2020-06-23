#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBTIMERINTERFACE_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBTIMERINTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "../usb_config.h"
#include "../m-stack/include/usb.h"
#include "../m-stack/include/usb_ch9.h"
#include "../m-stack/include/usb_hid.h"
#include "../Usb.h"

extern int16_t usbTimerReportLengthFor(uint8_t reportId);
extern uint8_t *usbTimerGetOutputReportBufferFor(uint8_t reportId);
extern int8_t usbTimerOnReportReceived(uint8_t endpoint, bool dataOk, void *context);

extern uint8_t usbTimerReportBuffer[];

#endif
