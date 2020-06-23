#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "UsbUartInterface.h"

uint8_t usbUartReportBuffer[8];

int16_t usbUartReportLengthFor(uint8_t reportId)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}

uint8_t *usbUartGetOutputReportBufferFor(uint8_t reportId)
{
	return (uint8_t *) usbUartReportBuffer;
}

int8_t usbUartOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}
