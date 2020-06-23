#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBUSBINTERFACE_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_PIN_USBUSBINTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "../usb_config.h"
#include "../m-stack/include/usb.h"
#include "../m-stack/include/usb_ch9.h"
#include "../m-stack/include/usb_hid.h"
#include "../Usb.h"
#include "../../FreeRtos.h"
#include "../../Pins.h"

#define USB_CONFIGURATION_NAME_DESCRIPTOR_REPORT_ID 0x04
#define USB_POWER_CONFIGURATION_REPORT_ID 0x08
#define USB_I2C_SEND_TO_HOST_REPORT_ID 0x10

struct UsbUsbAssignedPin
{
	struct Pin pin;
	uint16_t mask;
	uint16_t maskXor;
	uint16_t currentMilliamps;
	union
	{
		struct
		{
			unsigned int isStatusFlagOutput : 1;
			unsigned int isAnd : 1;
			unsigned int isActiveHigh : 1;
			unsigned int : 5;
		} bits;
		uint8_t all;
	} flags;
};

extern struct UsbUsbAssignedPin usbUsbAssignedPins[PINS_NUMBER_CONFIGURABLE];
extern QueueSetHandle_t usbUsbQueueSet;
extern QueueHandle_t usbUsbReportsQueue;

extern void usbUsbInitialise(void);
extern void usbUsbReportsAndEventsTask(void *args);
extern int16_t usbUsbReportLengthFor(uint8_t reportId);
extern uint8_t *usbUsbGetOutputReportBufferFor(uint8_t reportId);
extern int8_t usbUsbOnReportReceived(uint8_t endpoint, bool dataOk, void *context);
extern uint8_t *usbUsbGetInputReportFor(uint8_t reportType, uint8_t reportId);
extern uint32_t usbUsbGetStatusFlags(void);
extern uint32_t usbUsbGetCurrentLimitMilliamps(void);

extern bool usbUsbAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
extern void usbUsbUnassignPin(const struct Pin pin);

extern int16_t usbUsbGetConfigurationNameDescriptorReport(uint8_t *report);
extern void usbUsbOnConfigurationNameDescriptorReportReceived(const uint8_t *report);

extern int16_t usbUsbGetPowerConfigurationReport(uint8_t *report);
extern void usbUsbOnPowerConfigurationReportReceived(const uint8_t *report);

#endif
