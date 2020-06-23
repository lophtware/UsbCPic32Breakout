#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "UsbTimerInterface.h"

uint8_t usbTimerReportBuffer[8];

int16_t usbTimerReportLengthFor(uint8_t reportId)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}

uint8_t *usbTimerGetOutputReportBufferFor(uint8_t reportId)
{
	return (uint8_t *) usbTimerReportBuffer;
}

int8_t usbTimerOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}
