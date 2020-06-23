#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "UsbSpiInterface.h"

#define CLOCK_FUNCTION 0x01
#define MISO_FUNCTION 0x02
#define MOSI_FUNCTION 0x03
#define FRAME_SELECT_FUNCTION 0x04
#define SLAVE_SELECT_FUNCTION_MASK 0x08

static bool usbSpiAssignClockPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
static void usbSpiAssignPinToPpsOutput(struct PinState *pinState, uint8_t pinBehaviour, uint8_t ppsFunction);
static bool usbSpiAssignMisoPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
static bool usbSpiAssignMosiPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
static bool usbSpiAssignFrameSelectPin(struct PinState *pinState, const struct Pin pin, uint64_t args);
static bool usbSpiAssignSlaveSelectPin(struct PinState *pinState, const struct Pin pin, uint64_t args);

bool usbSpiAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (!pinState)
		return false;

	uint8_t function = (((uint8_t) (args >> 0)) & 0xff);
	if (function == CLOCK_FUNCTION)
		return usbSpiAssignClockPin(pinState, pin, args);

	if (function == MISO_FUNCTION)
		return usbSpiAssignMisoPin(pinState, pin, args);

	if (function == MOSI_FUNCTION)
		return usbSpiAssignMosiPin(pinState, pin, args);

	if (function == FRAME_SELECT_FUNCTION)
		return usbSpiAssignFrameSelectPin(pinState, pin, args);

	if (function & SLAVE_SELECT_FUNCTION_MASK)
		return usbSpiAssignSlaveSelectPin(pinState, pin, args);

	return false;
}

static bool usbSpiAssignClockPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (args & 0xffffffffffff0000ull)
		return false;

	uint8_t pinBehaviour = (((uint8_t) (args >> 8)) & 0xff);
	usbSpiAssignPinToPpsOutput(pinState, pinBehaviour, 9);
	return true;
}

static void usbSpiAssignPinToPpsOutput(struct PinState *pinState, uint8_t pinBehaviour, uint8_t ppsFunction)
{
	pinState->bits.ansel = 0;
	pinState->bits.cncon = 0;
	pinState->bits.cnen0 = 0;
	pinState->bits.cnen1 = 0;
	pinState->bits.cnenIsContinuous = 0;
	pinState->bits.cnpd = ((pinBehaviour & 0x03) == 1) ? 1 : 0;
	pinState->bits.cnpu = (pinBehaviour & 0x02) ? 1 : 0;
	pinState->bits.lat = (pinBehaviour & 0x08) ? 1 : 0;
	pinState->bits.odc = (pinBehaviour & 0x04) ? 1 : 0;
	pinState->bits.port = 0;
	pinState->bits.tris = 0;
	pinState->rpinr.raw = 0;
	pinState->rpor = ppsFunction;
}

static bool usbSpiAssignMisoPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (args & 0xffffffffffff0000ull)
		return false;

	uint8_t pinBehaviour = (((uint8_t) (args >> 8)) & 0xff);
	pinState->bits.ansel = 0;
	pinState->bits.cncon = 0;
	pinState->bits.cnen0 = 0;
	pinState->bits.cnen1 = 0;
	pinState->bits.cnenIsContinuous = 0;
	pinState->bits.cnpd = ((pinBehaviour & 0x03) == 1) ? 1 : 0;
	pinState->bits.cnpu = (pinBehaviour & 0x02) ? 1 : 0;
	pinState->bits.lat = 0;
	pinState->bits.odc = 0;
	pinState->bits.port = 0;
	pinState->bits.tris = 1;
	pinState->rpinr.raw = 0;
	pinState->rpinr.peripherals.sdi2 = 1;
	pinState->rpor = 0;

	return true;
}

static bool usbSpiAssignMosiPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (args & 0xffffffffffff0000ull)
		return false;

	uint8_t pinBehaviour = (((uint8_t) (args >> 8)) & 0xff);
	usbSpiAssignPinToPpsOutput(pinState, pinBehaviour, 8);
	return true;
}

static bool usbSpiAssignFrameSelectPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (args & 0xffffffffffff0000ull)
		return false;

	uint8_t pinBehaviour = (((uint8_t) (args >> 8)) & 0xff);
	usbSpiAssignPinToPpsOutput(pinState, pinBehaviour, 10);
	return true;
}

static bool usbSpiAssignSlaveSelectPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (args & 0xffffffffffff0000ull)
		return false;

	uint8_t slaveIndex = (((uint8_t) (args >> 0)) & 0x07);
	uint8_t pinBehaviour = (((uint8_t) (args >> 8)) & 0xff);
	usbSpiAssignPinToPpsOutput(pinState, pinBehaviour, 0);

	struct SpiSlaveSelectPin ssPin =
	{
		.handle = pin,
		.isActiveHigh = pinState->bits.lat ? false : true
	};

	spiSlavesAssignSelectPin(slaveIndex, ssPin);
	return true;
}

void usbSpiUnassignPin(const struct Pin pin)
{
	spiSlavesUnassignSelectPin(pin);
}
