#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "UsbI2cInterface.h"

#define IS_PIN_SCL(pin) ((pin).bank == 1 && (pin).index == 3)
#define IS_PIN_SDA(pin) ((pin).bank == 1 && (pin).index == 2)

#define IS_PIN_SCL_OR_SDA(pin) (IS_PIN_SCL(pin) || IS_PIN_SDA(pin))

static bool isSclAssigned = false;
static bool isSdaAssigned = false;

bool usbI2cAssignPin(struct PinState *pinState, const struct Pin pin, uint64_t args)
{
	if (!pinState || !IS_PIN_SCL_OR_SDA(pin))
		return false;

	if (args != 0)
		return false;

	pinState->bits.ansel = 0;
	pinState->bits.cncon = 0;
	pinState->bits.cnen0 = 0;
	pinState->bits.cnen1 = 0;
	pinState->bits.cnenIsContinuous = 0;
	pinState->bits.cnpd = 0;
	pinState->bits.cnpu = 0;
	pinState->bits.lat = 1;
	pinState->bits.odc = 1;
	pinState->bits.port = 0;
	pinState->bits.tris = 1;
	pinState->rpinr.raw = 0;
	pinState->rpor = 0;

	isSclAssigned = isSclAssigned || IS_PIN_SCL(pin);
	isSdaAssigned = isSdaAssigned || IS_PIN_SDA(pin);

	if (isSclAssigned && isSdaAssigned)
		i2cEnable();

	return true;
}

void usbI2cUnassignPin(const struct Pin pin)
{
	if (!IS_PIN_SCL_OR_SDA(pin))
		return;

	isSclAssigned = isSclAssigned && !IS_PIN_SCL(pin);
	isSdaAssigned = isSdaAssigned && !IS_PIN_SDA(pin);

	i2cDisable();
}
