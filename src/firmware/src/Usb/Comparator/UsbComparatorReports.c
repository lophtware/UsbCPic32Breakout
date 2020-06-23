#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "UsbComparatorInterface.h"

uint8_t usbComparatorReportBuffer[8];

int16_t usbComparatorReportLengthFor(uint8_t reportId)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}

uint8_t *usbComparatorGetOutputReportBufferFor(uint8_t reportId)
{
	return (uint8_t *) usbComparatorReportBuffer;
}

int8_t usbComparatorOnReportReceived(uint8_t endpoint, bool dataOk, void *context)
{
	// TODO: REPORTS NEED DEFINING
	return MSTACK_STALL;
}
