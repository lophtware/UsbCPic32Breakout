#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../Fault.h"
#include "../UsbCc.h"
#include "../FreeRtos.h"

#include "Usb.h"

#include "usb_config.h"
#include "m-stack/include/usb.h"

const struct UsbInterface *const usbInterfaces[NUMBER_OF_INTERFACES] =
{
	&usbCoreInterface,
	&usbTimerInterface,
	&usbCcpInterface,
	&usbUsbInterface,
	&usbI2cInterface,
	&usbUartInterface,
	&usbSpiInterface,
	&usbAdcInterface,
	&usbDacInterface,
	&usbComparatorInterface,
	&usbClcInterface
};

static void usbCheckForShortedDataLines(void);
static inline void usbWaitForOneMillisecondOrActivity(void);

static bool usbDataLinesShortedCheckPerformed = false;
static bool usbDataLinesAreShorted = false;

uint32_t usbInterruptHasBeenCalled;

void usbInitialise(QueueHandle_t flagsEventQueue, QueueHandle_t i2cEventQueue)
{
	if (!flagsEventQueue)
		faultUnrecoverableInitialisationError();

	if (!i2cEventQueue)
		faultUnrecoverableInitialisationError();

	usbFlagsEventQueue = flagsEventQueue;
	usbI2cEventQueue = i2cEventQueue;
	usbConfigurationInitialise();
	usbDescriptorsInitialise();
	usbSmallReportInitialise();
	for (int i = 0; i < NUMBER_OF_INTERFACES; i++)
	{
		if (usbInterfaces[i]->initialise)
			usbInterfaces[i]->initialise();
	}
}

void usbInitialiseAfterUsbCc(void)
{
	while (!usbCcIsInitialised())
		vTaskDelay(1);

	IPC7bits.USBIP = 3;
	IPC7bits.USBIS = 0;
	usb_init();
	usbCheckForShortedDataLines();
	usbConfigurationSet(configurationGetBootIndex());
}

static void usbCheckForShortedDataLines(void)
{
	usbDataLinesAreShorted = false;
	usbDataLinesShortedCheckPerformed = usbCurrentConfiguration.isUsbShortedDataLineTestEnabled;
	if (!usbDataLinesShortedCheckPerformed)
		return;

	for (int i = 0; i < 5; i++)
	{
		if (U1CONbits.JSTATE || U1OTGIRbits.ACTVIF || U1CONbits.SE0)
			break;

		usbWaitForOneMillisecondOrActivity();
	}

	usbDataLinesAreShorted = (U1CONbits.JSTATE || U1OTGIRbits.ACTVIF || U1CONbits.SE0) ? false : true;
}

static inline void usbWaitForOneMillisecondOrActivity(void)
{
	U1OTGIRCLR = _U1OTGIR_T1MSECIF_MASK;
	while (!U1OTGIRbits.T1MSECIF && !U1OTGIRbits.ACTVIF)
		;;
}

bool usbIsAttachedToDedicatedChargingPort(void)
{
	return !usbInterruptHasBeenCalled && usbDataLinesAreShorted && !U1OTGIRbits.ACTVIF;
}

bool usbWasDedicatedChargingPortTestPerformed(void)
{
	return usbDataLinesShortedCheckPerformed;
}
