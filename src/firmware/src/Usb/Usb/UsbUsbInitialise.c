#include <xc.h>
#include <stdint.h>

#include "../../Pins.h"
#include "../../FreeRtos.h"

#include "UsbUsbInterface.h"

struct UsbUsbAssignedPin usbUsbAssignedPins[PINS_NUMBER_CONFIGURABLE];
QueueSetHandle_t usbUsbQueueSet;

void usbUsbInitialise(void)
{
	for (int i = 0; i < PINS_NUMBER_CONFIGURABLE; i++)
	{
		usbUsbAssignedPins[i].pin.bank = pinsConfigurable[i].bank;
		usbUsbAssignedPins[i].pin.index = pinsConfigurable[i].index;
		usbUsbAssignedPins[i].flags.all = 0;
	}

	usbUsbReportsQueue = xQueueCreate(4, sizeof(uint8_t *));
	usbUsbQueueSet = xQueueCreateSet(uxQueueSpacesAvailable(usbUsbReportsQueue) + uxQueueSpacesAvailable(usbFlagsEventQueue) + 1);
	xQueueAddToSet(usbUsbReportsQueue, usbUsbQueueSet);
	xQueueAddToSet(usbFlagsEventQueue, usbUsbQueueSet);
	xTaskCreate(&usbUsbReportsAndEventsTask, "USB-USB", RESERVE_STACK_USAGE_BYTES(736), NULL, 3, NULL);
}
