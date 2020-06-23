#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "UsbAdcInterface.h"

uint8_t usbAdcReportBuffer[8];

int16_t usbAdcReportLengthFor(uint8_t reportId)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}

uint8_t *usbAdcGetOutputReportBufferFor(uint8_t reportId)
{
	return (uint8_t *) usbAdcReportBuffer;
}

int8_t usbAdcOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}
