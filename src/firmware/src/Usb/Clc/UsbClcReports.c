#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "UsbClcInterface.h"

uint8_t usbClcReportBuffer[8];

int16_t usbClcReportLengthFor(uint8_t reportId)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}

uint8_t *usbClcGetOutputReportBufferFor(uint8_t reportId)
{
	return (uint8_t *) usbClcReportBuffer;
}

int8_t usbClcOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}
