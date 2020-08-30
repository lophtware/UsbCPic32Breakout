#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_H
#include <stdint.h>
#include <stdbool.h>
#include "FreeRtos.h"
#include "EventQueueHeader.h"

#define USBCC_MODULE_ID 0x02
#define USBCC_EVENT_FLAGS_CHANGED 0x01

struct UsbCcEvent
{
	struct EventQueueHeader header;

	union
	{
		uint8_t raw[6];

		struct
		{
			union UsbCcEventFlags
			{
				struct
				{
					unsigned int isAttached : 1;
					unsigned int isDebugAccessory : 1;
					unsigned int isAudioAccessory : 1;
					unsigned int isActiveCable : 1;
					unsigned int isVbusVoltageOk : 1;
					unsigned int isCcVoltageOk : 1;
					unsigned int isOrientationCc1A5 : 1;
					unsigned int isOrientationCc2B5 : 1;
					unsigned int isOrientationOk : 1;
					unsigned int currentLimit : 2;
					unsigned int : 5;
				} bits;
				uint16_t all;
			} flags;
		} flagsChanged;
	} as;
};

extern void usbCcInitialise(QueueHandle_t eventQueue);
extern bool usbCcIsInitialised(void);
extern uint16_t usbCcGetCurrentLimitMilliamps(void);

#endif
