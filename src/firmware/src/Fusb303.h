#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FUSB303_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_FUSB303_H
#include <stdint.h>
#include <stdbool.h>
#include "FreeRtos.h"
#include "EventQueueHeader.h"

#define FUSB303_MODULE_ID 0x02
#define FUSB303_EVENT_FLAGS_CHANGED 0x01

struct Fusb303Event
{
	struct EventQueueHeader header;

	union
	{
		uint8_t raw[6];

		struct
		{
			union Fusb303EventFlags
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

extern void fusb303Initialise(QueueHandle_t eventQueue);
extern bool fusb303IsInitialised(void);
extern uint16_t fusb303GetCurrentLimitMilliamps(void);

#endif
