#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "Usb.h"

#include "usb_config.h"
#include "m-stack/include/usb.h"
#include "m-stack/include/usb_ch9.h"
#include "m-stack/include/usb_hid.h"

#define REMOTE_WAKEUP_ENABLED 0x02

static const struct UsbEvent usbFlagsEvaluationChangedEvent =
{
	.header = EVENT_QUEUE_HEADER_INIT(USB_MODULE_ID, USB_EVENT_FLAGS_EVALUATION_CHANGED)
};

static uint32_t usbHaltedEndpoints;

uint32_t usbGetHaltedEndpointsAsMask(void)
{
	return usbHaltedEndpoints;
}

void usbOnReset(void)
{
	usbHaltedEndpoints = 0;
}

uint16_t usbOnGetDeviceStatus(void)
{
	return 0;// return REMOTE_WAKEUP_ENABLED; // TODO: WE SAY 'YES' BUT M-STACK DOESN'T SUPPORT IT, SO MODIFY M-STACK ACCORDINGLY - NOTE THAT THE HOST CAN ENABLE / DISABLE REMOTE WAKEUP, SO TAKE THAT INTO ACCOUNT AS WELL
}

void usbOnEndpointHalt(uint8_t endpoint, bool halted)
{
	UBaseType_t criticalHandle = taskENTER_CRITICAL_FROM_ISR();
	if (halted)
		usbHaltedEndpoints |= (1 << endpoint);
	else
		usbHaltedEndpoints &= ~(1 << endpoint);

	taskEXIT_CRITICAL_FROM_ISR(criticalHandle);

	xQueueSendToBackFromISR(usbFlagsEventQueue, &usbFlagsEvaluationChangedEvent, NULL);
}

void usbOnEndpointHaltFromNonIsr(uint8_t endpoint, bool halted)
{
	taskENTER_CRITICAL();
	if (halted)
		usbHaltedEndpoints |= (1 << endpoint);
	else
		usbHaltedEndpoints &= ~(1 << endpoint);

	taskEXIT_CRITICAL();

	while (xQueueSendToBack(usbFlagsEventQueue, &usbFlagsEvaluationChangedEvent, portMAX_DELAY) != pdPASS)
	{
		vTaskDelay(1);
	}
}

int8_t usbOnSetInterface(uint8_t interface, uint8_t altSetting)
{
	return MSTACK_STALL;
}

int8_t usbOnGetInterface(uint8_t interface)
{
	return MSTACK_STALL;
}

int8_t usbOnUnknownSetupRequest(const struct setup_packet *setup)
{
	return process_hid_setup_request(setup);
}

int16_t usbOnGetUnknownDescriptor(const struct setup_packet *packet, const void **descriptor)
{
	uint8_t type = (uint8_t) ((packet->wValue >> 8) & 0x00ff);
	if (type == DESC_HID)
		return usbHidOnGetDescriptor(packet->wIndex, descriptor);

	if (type == DESC_REPORT)
		return usbHidOnGetReportDescriptor(packet->wIndex, descriptor);

	return MSTACK_STALL;
}
