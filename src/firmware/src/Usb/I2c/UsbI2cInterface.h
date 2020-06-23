#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBI2CINTERFACE_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBI2CINTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "../usb_config.h"
#include "../m-stack/include/usb.h"
#include "../m-stack/include/usb_ch9.h"
#include "../m-stack/include/usb_hid.h"
#include "../Usb.h"
#include "../../FreeRtos.h"

#define I2C_BUS_CONFIGURATION_REPORT_ID 0x04
#define I2C_MASTER_STORED_CONFIGURATION_REPORT_ID 0x06
#define I2C_MASTER_IMMEDIATE_CONFIGURATION_REPORT_ID (I2C_MASTER_STORED_CONFIGURATION_REPORT_ID | 1)
#define I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID 0x08
#define I2C_SLAVE_IMMEDIATE_CONFIGURATION_REPORT_ID (I2C_SLAVE_STORED_CONFIGURATION_REPORT_ID | 1)
#define I2C_SLAVE_REPORT_CONFIGURATION_REPORT_ID 0x0a
#define I2C_SLAVE_TRANSACTION_DONE_REPORT_ID 0x0c
#define I2C_PROTECTED_RAM_ADDRESS_MASK_REPORT_ID 0x10
#define I2C_RAM_TRANSACTION_REPORT_ID 0x11
#define I2C_RAM_BANK_WRITE_PROTECT_FLAG_REPORT_ID 0x14
#define I2C_RAM_INITIALISATION_CONTENTS_REPORT_ID 0x16
#define I2C_PROTECTED_ROM_ADDRESS_MASK_REPORT_ID 0x18
#define I2C_ROM_TRANSACTION_REPORT_ID 0x19
#define I2C_ROM_BANK_WRITE_PROTECT_FLAG_REPORT_ID 0x1c
#define I2C_ROM_CONTENTS_REPORT_ID 0x1e
#define I2C_TRANSACTION_REPORT_FLAG 0x80

extern QueueSetHandle_t usbI2cQueueSet;
extern QueueHandle_t usbI2cReportsQueue;

extern void usbI2cInitialise(void);
extern void usbI2cReportsAndEventsTask(void *args);
extern int16_t usbI2cReportLengthFor(uint8_t reportId);
extern uint8_t *usbI2cGetOutputReportBufferFor(uint8_t reportId);
extern int8_t usbI2cOnReportReceived(uint8_t endpoint, bool dataOk, void *context);
extern uint8_t *usbI2cGetInputReportFor(uint8_t reportType, uint8_t reportId);

extern bool usbI2cAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
extern void usbI2cUnassignPin(const struct Pin pin);

extern int16_t usbI2cGetBusConfigurationReport(uint8_t *report);
extern void usbI2cOnBusConfigurationReportReceived(const uint8_t *report);

extern int16_t usbI2cGetMasterStoredConfigurationReport(uint8_t *report);
extern int16_t usbI2cGetMasterImmediateConfigurationReport(uint8_t *report);
extern void usbI2cOnMasterConfigurationReportReceived(const uint8_t *report);

extern int16_t usbI2cGetSlaveStoredConfigurationReport(uint8_t *report);
extern int16_t usbI2cGetSlaveImmediateConfigurationReport(uint8_t *report);
extern void usbI2cOnSlaveConfigurationReportReceived(const uint8_t *report);

extern int16_t usbI2cGetSlaveReportConfigurationReport(uint8_t *report);
extern void usbI2cOnSlaveReportConfigurationReportReceived(const uint8_t *report);

extern int16_t usbI2cGetProtectedRamAddressMaskReport(uint8_t *report);
extern void usbI2cOnProtectedRamAddressMaskReportReceived(const uint8_t *report);

extern int16_t usbI2cGetRamInitialisationContentsReport(uint8_t *report);
extern void usbI2cOnRamInitialisationContentsReportReceived(const uint8_t *report);

extern int16_t usbI2cGetProtectedRomAddressMaskReport(uint8_t *report);
extern void usbI2cOnProtectedRomAddressMaskReportReceived(const uint8_t *report);

extern int16_t usbI2cGetRomBankWriteProtectFlagReport(uint8_t *report);
extern void usbI2cOnRomBankWriteProtectFlagReportReceived(const uint8_t *report);

extern int16_t usbI2cGetRamBankWriteProtectFlagReport(uint8_t *report);
extern void usbI2cOnRamBankWriteProtectFlagReportReceived(const uint8_t *report);

extern void usbI2cOnTransactionReportReceived(const uint8_t *report);

extern int16_t usbI2cGetRomContentsReport(uint8_t *report);
extern void usbI2cOnRomContentsReportReceived(const uint8_t *report);

extern void usbI2cOnRomAndRamTransactionReportReceived(const uint8_t *report);

#endif
