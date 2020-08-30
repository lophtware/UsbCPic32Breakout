#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../FreeRtos.h"
#include "../Syskey.h"
#include "../Pins.h"

#include "BenchTesting.h"

static void benchTestingUnlockPeripheralPinSelect(void);
static void benchTestingLockPeripheralPinSelect(void);

void benchTestingToggleAllPinsForSeconds(uint16_t seconds)
{
	struct BenchTestingCommand command =
	{
		.as =
		{
			.fields =
			{
				.type = COMMAND_TOGGLE_ALL_PINS,
				.args = {(uint8_t) (seconds & 0xff), (uint8_t) ((seconds >> 8) & 0xff), 0}
			}
		}
	};

	benchTestingSendRawCommand(command.as.raw);
}

void benchTestingTaskToggleAllPinsForSeconds(uint16_t seconds)
{
	TickType_t duration = pdMS_TO_TICKS(seconds * 1000);

	LATACLR = CONFIGURABLE_A_MASK;
	LATBCLR = CONFIGURABLE_B_MASK;
	LATCCLR = CONFIGURABLE_C_MASK;

	TickType_t startTicks = xTaskGetTickCount();
	while ((xTaskGetTickCount() - startTicks) < duration)
	{
		LATAINV = CONFIGURABLE_A_MASK;
		LATBINV = CONFIGURABLE_B_MASK;
		LATCINV = CONFIGURABLE_C_MASK;
	}

	LATACLR = CONFIGURABLE_A_MASK;
	LATBCLR = CONFIGURABLE_B_MASK;
	LATCCLR = CONFIGURABLE_C_MASK;
}

void benchTestingToggleAllPinsWithPwmForSeconds(uint16_t seconds, uint8_t period)
{
	struct BenchTestingCommand command =
	{
		.as =
		{
			.fields =
			{
				.type = COMMAND_TOGGLE_ALL_PINS_WITH_PWM,
				.args = {(uint8_t) (seconds & 0xff), (uint8_t) ((seconds >> 8) & 0xff), period}
			}
		}
	};

	benchTestingSendRawCommand(command.as.raw);
}

void benchTestingTaskToggleAllPinsWithPwmForSeconds(uint16_t seconds, uint8_t period)
{
	CCP4CON1bits.CCSEL = 0;
	CCP4CON1bits.MOD = 5;
	CCP4CON1bits.T32 = 0;
	CCP4CON1bits.TMRSYNC = 0;
	CCP4CON1bits.CLKSEL = 0;
	CCP4CON1bits.TMRPS = 0;
	CCP4CON1bits.TRIGEN = 0;
	CCP4CON1bits.SYNC = 0;
	CCP4CON2bits.OCAEN = 1;
	CCP4CON3bits.POLACE = 0;
	CCP4TMRbits.TMRL = 0;
	CCP4PRbits.PRL = period;
	CCP4RA = 0;
	CCP4RB = period;

	syskeyUnlockThen(benchTestingUnlockPeripheralPinSelect);
	static const int ccp4Output = 11;
	RPOR0bits.RP3R = ccp4Output;
	RPOR0bits.RP4R = ccp4Output;
	RPOR1bits.RP5R = ccp4Output;
	RPOR1bits.RP6R = ccp4Output;
	RPOR1bits.RP7R = ccp4Output;
	RPOR1bits.RP8R = ccp4Output;
	RPOR2bits.RP9R = ccp4Output;
	RPOR2bits.RP10R = ccp4Output;
	RPOR2bits.RP11R = ccp4Output;
	RPOR2bits.RP12R = ccp4Output;
	RPOR4bits.RP18R = ccp4Output;

#ifdef BOARD_VARIANT_LITE
	RPOR0bits.RP1R = ccp4Output;
	RPOR0bits.RP2R = ccp4Output;
	RPOR3bits.RP13R = ccp4Output;
	RPOR3bits.RP14R = ccp4Output;
#endif

	syskeyUnlockThen(benchTestingLockPeripheralPinSelect);

	LATACLR = CONFIGURABLE_A_MASK;
	LATBCLR = CONFIGURABLE_B_MASK;
	LATCCLR = CONFIGURABLE_C_MASK;

	CCP4CON1bits.ON = 1;

	TickType_t duration = pdMS_TO_TICKS(seconds * 1000);
	vTaskDelay(duration);

	CCP4CON1bits.ON = 0;

	LATACLR = CONFIGURABLE_A_MASK;
	LATBCLR = CONFIGURABLE_B_MASK;
	LATCCLR = CONFIGURABLE_C_MASK;
}

static void benchTestingUnlockPeripheralPinSelect(void)
{
	RPCONCLR = _RPCON_IOLOCK_MASK;
}

static void benchTestingLockPeripheralPinSelect(void)
{
	RPCONSET = _RPCON_IOLOCK_MASK;
}
