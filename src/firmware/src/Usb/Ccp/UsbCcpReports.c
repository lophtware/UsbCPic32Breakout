#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "UsbCcpInterface.h"

uint8_t usbCcpReportBuffer[8];

int16_t usbCcpReportLengthFor(uint8_t reportId)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}

uint8_t *usbCcpGetOutputReportBufferFor(uint8_t reportId)
{
	return (uint8_t *) usbCcpReportBuffer;
}

int8_t usbCcpOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}
