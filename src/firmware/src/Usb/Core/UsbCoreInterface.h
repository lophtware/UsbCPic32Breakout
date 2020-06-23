#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBCOREINTERFACE_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBCOREINTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "../usb_config.h"
#include "../m-stack/include/usb.h"
#include "../m-stack/include/usb_ch9.h"
#include "../m-stack/include/usb_hid.h"
#include "../Usb.h"
#include "../../FreeRtos.h"

#define CORE_STATUS_REPORT_ID 0x02
#define CORE_SET_UNLOCK_KEY_REPORT_ID 0x04
#define CORE_STORE_CONFIGURATION_REPORT_ID 0x08
#define CORE_PIN_LAT_REPORT_ID 0x14
#define CORE_PIN_STATUS_REPORT_ID 0x16
#define CORE_PINS_CHANGED_REPORT_ID 0x18
#define CORE_PINS_CHANGED_RESET_REPORT_ID 0x19
#define CORE_PIN_CONFIGURATION_REPORT_ID_MASK 0xc0
#define CORE_PIN_CONFIGURATION_REPORT_ID 0x40
#define CORE_BENCH_TESTING_REPORT_ID 0xff

#define REPORT_NACK_UNKNOWN_INTERFACE(intf) REPORT_NACK_IMPLEMENTATION1(0x01, (intf))
#define REPORT_NACK_UNKNOWN_PIN(pin) REPORT_NACK_IMPLEMENTATION1(0x02, (pin))
#define REPORT_NACK_UNKNOWN_FUNCTION(func) REPORT_NACK_IMPLEMENTATION1(0x03, (func))
#define REPORT_NACK_INVALID_CONFIGURATION REPORT_NACK_IMPLEMENTATION(0x10)

#define PIN_STATUS_REPORT_FLAG_RESPONSE 0x01

extern QueueHandle_t usbCoreReportsQueue;

extern void usbCoreInitialise(void);
extern void usbCoreReportsTask(void *args);
extern int16_t usbCoreReportLengthFor(uint8_t reportId);
extern uint8_t *usbCoreGetOutputReportBufferFor(uint8_t reportId);
extern int8_t usbCoreOnReportReceived(uint8_t endpoint, bool dataOk, void *context);
extern uint8_t *usbCoreGetInputReportFor(uint8_t reportType, uint8_t reportId);

extern int16_t usbCoreGetStatusReport(uint8_t *report);

extern void usbCoreOnSetUnlockKeyReportReceived(const uint8_t *report);

extern void usbCoreOnStoreConfigurationReportReceived(const uint8_t *report);

extern void usbCorePinsTask(void *args);

extern void usbCoreOnPinConfigurationReportReceived(const uint8_t *report);
extern bool usbCoreAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
extern int16_t usbCoreGetPinConfigurationReport(uint8_t *report);

extern void usbCoreOnPinLatReportReceived(const uint8_t *report);
extern int16_t usbCoreGetPinStatusReport(uint8_t *report);
extern void usbCoreOnPinsChangedResetReportReceived(const uint8_t *report);

#endif
