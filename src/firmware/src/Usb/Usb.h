#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_USB_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USB_USB_H
#include <stdint.h>
#include <stdbool.h>
#include "usb_config.h"
#include "m-stack/include/usb.h"
#include "m-stack/include/usb_ch9.h"
#include "m-stack/include/usb_hid.h"
#include "../Configuration.h"
#include "../Pins.h"
#include "../EventQueueHeader.h"
#include "../Usb.h"
#include "../FreeRtos.h"

#define MSTACK_STALL -1
#define MSTACK_NAK -2

#define DEFINE_USB_INTERFACE(n) \
__attribute__((packed)) struct UsbInterface_##n \
{ \
	struct interface_descriptor interface; \
	struct hid_descriptor hid; \
	struct hid_optional_descriptor reports[n - 1]; \
}

#define UsbInterface(n) UsbInterface_##n

#define DEFINE_USB_IN_INTERFACE(n) \
__attribute__((packed)) struct UsbInInterface_##n \
{ \
	struct interface_descriptor interface; \
	struct hid_descriptor hid; \
	struct hid_optional_descriptor reports[n - 1]; \
	struct endpoint_descriptor ep_in; \
}

#define UsbInInterface(n) UsbInInterface_##n

#define DEFINE_USB_IN_OUT_INTERFACE(n) \
__attribute__((packed)) struct UsbInOutInterface_##n \
{ \
	struct interface_descriptor interface; \
	struct hid_descriptor hid; \
	struct hid_optional_descriptor reports[n - 1]; \
	struct endpoint_descriptor ep_in; \
	struct endpoint_descriptor ep_out; \
}

#define UsbInOutInterface(n) UsbInOutInterface_##n

DEFINE_USB_INTERFACE(1);
DEFINE_USB_IN_INTERFACE(1);
DEFINE_USB_IN_OUT_INTERFACE(1);

#define __STRING_DESCRIPTOR(name, ...) struct \
	{ \
		uint8_t bLength; \
		uint8_t bDescriptorType; \
		uint16_t chars[sizeof((uint16_t[]) {__VA_ARGS__}) / sizeof(uint16_t)]; \
	} name = \
	{ \
		.bLength = sizeof(name), \
		.bDescriptorType = DESC_STRING, \
		.chars = {__VA_ARGS__} \
	}

#define NON_CONST_STRING_DESCRIPTOR(name, ...) static __STRING_DESCRIPTOR(name, __VA_ARGS__)

#define STRING_DESCRIPTOR(name, ...) static const __STRING_DESCRIPTOR(name, __VA_ARGS__)

#define EP_IN 0x80
#define EP_OUT 0x00
#define LANGID_UK 0x0809
#define HID_VERSION 0x0111

#define INDEX_OF_INTERFACE_DESCRIPTOR(intf, index) (((intf##_INTERFACE_ID + 1) << 4) | (index))

#define USB_EVENT_CONFIGURATION_CHANGED 0x01
#define USB_EVENT_FLAGS_EVALUATION_CHANGED 0x02

#define USB_ACKNOWLEDGEMENT_REPORT_ID 0x01
#define REPORT_ACK_OK 0
#define REPORT_NACK_GENERIC(x) (0x80000000 | ((x) << 24))
#define REPORT_NACK_IMPLEMENTATION1(x, a) (0xc0000000 | ((x) << 24) | ((uint32_t) ((a) & 0xff) << 16))
#define REPORT_NACK_IMPLEMENTATION(x) REPORT_NACK_IMPLEMENTATION1(x, 0)
#define REPORT_NACK_UNKNOWN_ID REPORT_NACK_GENERIC(0x01)
#define REPORT_NACK_QUEUE_FULL REPORT_NACK_GENERIC(0x02)
#define REPORT_NACK_OUT_OF_BOUNDS REPORT_NACK_GENERIC(0x03)
#define REPORT_NACK_UNLOCK_KEY REPORT_NACK_GENERIC(0x10)

__attribute__((packed))
struct ConfigurationDescriptor
{
	struct configuration_descriptor configuration;
	struct UsbInOutInterface(1) coreInterface;

#ifdef TIMER_INTERFACE_ENABLED
	struct UsbInInterface(1) timerInterface;
#endif

#ifdef CCP_INTERFACE_ENABLED
	struct UsbInInterface(1) ccpInterface;
#endif

	struct UsbInOutInterface(1) usbInterface;

#ifdef I2C_INTERFACE_ENABLED
	struct UsbInOutInterface(1) i2cInterface;
#endif

#ifdef UART_INTERFACE_ENABLED
	struct UsbInOutInterface(1) uartInterface;
#endif

#ifdef SPI_INTERFACE_ENABLED
	struct UsbInOutInterface(1) spiInterface;
#endif

#ifdef ADC_INTERFACE_ENABLED
	struct UsbInInterface(1) adcInterface;
#endif

#ifdef DAC_INTERFACE_ENABLED
	struct UsbInterface(1) dacInterface;
#endif

#ifdef COMPARATOR_INTERFACE_ENABLED
	struct UsbInInterface(1) comparatorInterface;
#endif

#ifdef CLC_INTERFACE_ENABLED
	struct UsbInOutInterface(1) clcInterface;
#endif
};

typedef int8_t (*UsbEpDataStageCallback)(uint8_t ep, bool dataOk, void *context);

struct UsbInterface
{
	uint8_t endpoint;
	void (*initialise)(void);
	int16_t (*reportLengthFor)(uint8_t reportId);
	uint8_t *(*getOutputReportBufferFor)(uint8_t reportId);
	UsbEpDataStageCallback onReportReceived;
	uint8_t *(*getInputReportFor)(uint8_t reportType, uint8_t reportId);
	usb_ep0_data_stage_callback onReportSent;
	uint16_t (*memcpyInterfaceDescriptor)(void *dest);
	int16_t (*getHidDescriptor)(const void **ptr);
	int16_t (*getStringDescriptor)(uint8_t index, const void **ptr);
	int16_t (*getReportDescriptor)(const void **ptr);
	bool (*assignPin)(struct PinState *pinState, const struct Pin pin, uint64_t args);
	void (*unassignPin)(const struct Pin pin);
};

__attribute__((packed))
struct UsbEvent
{
	struct EventQueueHeader header;

	union
	{
		uint8_t raw[6];

		struct
		{
			uint8_t configurationIndex;
		} configurationChanged;
	} as;
};

__attribute__((packed))
struct UsbSmallReport
{
	union
	{
		uint32_t dwords[2];

		struct
		{
			uint8_t payload[7];
			struct
			{
				unsigned int endpoint : 4;
				unsigned int : 1;
				unsigned int count : 3;
			} flags;
		} fields;
	} as;
};

extern const struct configuration_descriptor *usbConfigurationDescriptors[NUMBER_OF_CONFIGURATIONS];
extern const struct ConfigurationDescriptor *usbCurrentConfigurationDescriptor;
extern struct Configuration usbCurrentConfiguration;

extern const struct UsbInterface usbCoreInterface;
extern const struct UsbInterface usbTimerInterface;
extern const struct UsbInterface usbCcpInterface;
extern const struct UsbInterface usbUsbInterface;
extern const struct UsbInterface usbI2cInterface;
extern const struct UsbInterface usbUartInterface;
extern const struct UsbInterface usbSpiInterface;
extern const struct UsbInterface usbAdcInterface;
extern const struct UsbInterface usbDacInterface;
extern const struct UsbInterface usbComparatorInterface;
extern const struct UsbInterface usbClcInterface;
extern const struct UsbInterface *const usbInterfaces[NUMBER_OF_INTERFACES];

extern QueueHandle_t usbFlagsEventQueue;
extern QueueHandle_t usbI2cEventQueue;

extern void usbDescriptorsInitialise(void);
extern bool usbDescriptorsSetConfigurationName(uint8_t descriptorIndex, const uint8_t *unicodeName);

extern void usbConfigurationInitialise(void);
extern void usbConfigurationSet(uint8_t configuration);
extern void usbConfigurationApply(const struct UsbEvent *event);
extern void usbInitialiseAfterFusb303(void);

extern uint8_t usbMapEndpointToInterface(uint8_t endpoint);
extern uint8_t usbMapInterfaceToEndpoint(uint8_t interface);

extern void usbSmallReportInitialise(void);
extern bool usbSmallReportSend(struct UsbSmallReport report);
extern bool usbSmallReportSendFromIsr(struct UsbSmallReport report, BaseType_t *wasHigherPriorityTaskWoken);
extern void usbSmallReportEnableFor(uint8_t endpoint);
extern void usbSmallReportDisableFor(uint8_t endpoint);

extern void usbStartReceiveEpDataStage(uint8_t endpoint, uint8_t *buffer, size_t length, UsbEpDataStageCallback callback, void *context);
extern bool usbStartSendEpDataStage(uint8_t endpoint, uint8_t *buffer, size_t length, UsbEpDataStageCallback callback, void *context);
extern bool usbIsInEndpointBusy(uint8_t endpoint);

extern void usbOnEndpointHaltFromNonIsr(uint8_t endpoint, bool halted);
extern uint32_t usbGetHaltedEndpointsAsMask(void);

#define _SMALL_REPORT(var, endpoint, reportId, payload) \
	struct UsbSmallReport var = \
	{ \
		.as = \
		{ \
			.fields = \
			{ \
				.payload = \
				{ \
					USB_ACKNOWLEDGEMENT_REPORT_ID, \
					reportId, \
					(uint8_t) ((payload >> 24) & 0xff), \
					(uint8_t) ((payload >> 16) & 0xff), \
					(uint8_t) ((payload >> 8) & 0xff), \
					(uint8_t) ((payload >> 0) & 0xff), \
					0x00 \
				}, \
				.flags = { .endpoint = endpoint, .count = 7 } \
			} \
		} \
	};

static inline void usbSendAcknowledgementReport(uint8_t endpoint, uint8_t reportId, uint32_t payload)
{
	_SMALL_REPORT(done, endpoint, reportId, payload);
	if (!usbSmallReportSend(done))
	{
		usb_halt_ep_out(endpoint);
		usbOnEndpointHaltFromNonIsr(endpoint, true);
	}
}

static inline void usbSendAcknowledgementReportFromIsr(uint8_t endpoint, uint8_t reportId, uint32_t payload)
{
	_SMALL_REPORT(done, endpoint, reportId, payload);
	if (!usbSmallReportSendFromIsr(done, NULL))
	{
		usb_halt_ep_out(endpoint);
		usbOnEndpointHalt(endpoint, true);
	}
}

#endif
