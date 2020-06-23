#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "UsbDacInterface.h"

uint8_t usbDacReportBuffer[8];

int16_t usbDacReportLengthFor(uint8_t reportId)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}

uint8_t *usbDacGetOutputReportBufferFor(uint8_t reportId)
{
	return (uint8_t *) usbDacReportBuffer;
}

int8_t usbDacOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}
