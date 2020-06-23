#include <xc.h>
#include <stdint.h>

#include "Usb.h"

#include "usb_config.h"
#include "m-stack/include/usb.h"
#include "m-stack/include/usb_ch9.h"

int16_t usbHidOnGetDescriptor(uint8_t interface, const void **ptr)
{
	if (interface >= NUMBER_OF_INTERFACES)
		return MSTACK_STALL;

	return usbInterfaces[interface]->getHidDescriptor(ptr);
}

int16_t usbHidOnGetReportDescriptor(uint8_t interface, const void **ptr)
{
	if (interface >= NUMBER_OF_INTERFACES)
		return MSTACK_STALL;

	return usbInterfaces[interface]->getReportDescriptor(ptr);
}

int16_t usbHidOnGetReport(
	uint8_t interface,
	uint8_t reportType,
	uint8_t reportId,
	const void **report,
	usb_ep0_data_stage_callback *callback,
	void **context)
{
	if (interface >= NUMBER_OF_INTERFACES)
		return MSTACK_STALL;

	const struct UsbInterface *interfaceModule = usbInterfaces[interface];
	int16_t reportLength = interfaceModule->reportLengthFor(reportId);
	if (reportLength <= 0)
		return MSTACK_STALL;

	*report = interfaceModule->getInputReportFor(reportType, reportId);
	if (!*report)
		return MSTACK_STALL; // TODO: THIS SHOULD PROBABLY BE 'NAK' ?  BUT THE NAK FUNCTIONALITY IS NOT AVAILABLE AT PRESENT.
		
	*context = (void *) 0;
	*callback = interfaceModule->onReportSent;
	return reportLength;
}

int8_t usbHidOnSetReport(uint8_t interface, uint8_t reportType, uint8_t reportId)
{
	if (interface >= NUMBER_OF_INTERFACES)
		return MSTACK_STALL;

	const struct UsbInterface *interfaceModule = usbInterfaces[interface];
	int16_t reportLength = interfaceModule->reportLengthFor(reportId);
	if (reportLength <= 0)
	{
		uint8_t endpoint = usbMapInterfaceToEndpoint(interface);
		usbSendAcknowledgementReportFromIsr(endpoint, reportId, REPORT_NACK_UNKNOWN_ID);
		return MSTACK_STALL;
	}

	uint8_t *reportBuffer = interfaceModule->getOutputReportBufferFor(reportId);
	if (!reportBuffer)
		return MSTACK_NAK;

	usbStartReceiveEpDataStage(
		interfaceModule->endpoint,
		reportBuffer,
		reportLength,
		interfaceModule->onReportReceived,
		reportBuffer);

	return 0;
}

uint8_t usbHidOnGetIdle(uint8_t interface, uint8_t reportId)
{
	return 0;
}

int8_t usbHidOnSetIdle(uint8_t interface, uint8_t reportId, uint8_t idleRate)
{
	return MSTACK_STALL;
}

int8_t usbHidOnGetProtocol(uint8_t interface)
{
	return MSTACK_STALL;
}

int8_t usbHidOnSetProtocol(uint8_t interface, uint8_t protocol)
{
	return MSTACK_STALL;
}
