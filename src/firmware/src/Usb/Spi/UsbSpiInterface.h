#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBSPIINTERFACE_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBSPIINTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "../usb_config.h"
#include "../m-stack/include/usb.h"
#include "../m-stack/include/usb_ch9.h"
#include "../m-stack/include/usb_hid.h"
#include "../Usb.h"
#include "../../FreeRtos.h"

#define SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(s) (0x10 | ((s) << 1))
#define SPI_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID(s) (SPI_SLAVE_STORED_CONFIGURATION_REPORT_ID(s) | 1)
#define SPI_TRANSACTION_REPORT_FLAG 0x80

extern uint8_t usbSpiTransactionReportBuffer[1028];
extern QueueHandle_t usbSpiReportsQueue;

extern void usbSpiInitialise(void);
extern void usbSpiReportsTask(void *args);
extern int16_t usbSpiReportLengthFor(uint8_t reportId);
extern uint8_t *usbSpiGetOutputReportBufferFor(uint8_t reportId);
extern int8_t usbSpiOnReportReceived(uint8_t endpoint, bool dataOk, void *context);
extern uint8_t *usbSpiGetInputReportFor(uint8_t reportType, uint8_t reportId);

extern bool usbSpiAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
extern void usbSpiUnassignPin(const struct Pin pin);

extern void usbSpiOnSlaveConfigurationReportReceived(const uint8_t *report);
extern int16_t usbSpiGetSlaveImmediateConfigurationReport(uint8_t *report);
extern int16_t usbSpiGetSlaveStoredConfigurationReport(uint8_t *report);

extern void usbSpiOnTransactionReportReceived(const uint8_t *report);

#endif
