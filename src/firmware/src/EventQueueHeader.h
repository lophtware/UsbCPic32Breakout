#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_EVENTQUEUEHEADER_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_EVENTQUEUEHEADER_H
#include <stdint.h>

#define EVENT_QUEUE_HEADER_INIT(moduleId, eventType) \
	{ \
		.as = { .raw = EVENT_QUEUE_HEADER_WORD_FOR(moduleId, eventType) } \
	}

#define EVENT_QUEUE_HEADER_WORD_FOR(moduleId, eventType) ((((uint16_t) (moduleId)) << 8) | (uint16_t) (eventType))

__attribute__((packed))
struct EventQueueHeader
{
	union
	{
		uint16_t raw;

		struct
		{
			uint8_t moduleId;
			uint8_t eventType;
		} fields;
	} as;
};

#endif
