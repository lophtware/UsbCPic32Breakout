#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "../Fault.h"
#include "../FreeRtos.h"

#include "Fusb303.h"

#if configPERIPHERAL_CLOCK_HZ != 24000000
#error The value of the I2C1BRG register will be wrong if the peripheral clock is not 24MHz
#endif

#define BRG_NO_MORE_THAN_400KHZ 29

extern void __attribute__((interrupt(), vector(_I2C1_MASTER_VECTOR), nomips16)) _i2c1MasterInterrupt(void);
extern void __attribute__((interrupt(), vector(_EXTERNAL_1_VECTOR), nomips16)) _int1PinInterrupt(void);

static void fusb303InitialiseI2cModule(void);
static void fusb303InitialiseNextRegister(const struct Fusb303Transaction *transaction);
static void fusb303OnRegisterInitialisationDone(void);

TaskHandle_t fusb303TaskHandle;
QueueHandle_t fusb303Transactions;
QueueHandle_t fusb303Events;

static bool isInitialised = false;

static const struct Fusb303Register fusb303RegisterInitialisation[] =
{
	{ .address = FUSB303_REG_PORTROLE, .value = FUSB303_REG_PORTROLE_ORIENTDEB | FUSB303_REG_PORTROLE_SNK },
	{ .address = FUSB303_REG_CONTROL, .value = FUSB303_REG_CONTROL_DCABLE_EN | FUSB303_REG_CONTROL_HOSTCUR_DEFAULT },
	{ .address = FUSB303_REG_CONTROL1, .value = FUSB303_REG_CONTROL1_ENABLE },
	{ .address = FUSB303_REG_MANUAL, .value = FUSB303_REG_MANUAL_UNATT_SNK },
	{ .address = FUSB303_REG_MASK, .value = FUSB303_REG_MASK_AUTOSNK },
	{ .address = FUSB303_REG_MASK1, .value = FUSB303_REG_MASK1_VBOFF | FUSB303_REG_MASK1_VBON | FUSB303_REG_MASK1_FRC_FAIL | FUSB303_REG_MASK1_FRC_SUCC },
	{ .address = FUSB303_REG_INTERRUPT, .value = FUSB303_REG_INTERRUPT_CLEARALL },
	{ .address = FUSB303_REG_INTERRUPT1, .value = FUSB303_REG_INTERRUPT1_CLEARALL }
};

void fusb303Initialise(QueueHandle_t eventQueue)
{
	if (!eventQueue)
		faultUnrecoverableInitialisationError();

	fusb303Events = eventQueue;
	fusb303Transactions = xQueueCreate(2, sizeof(struct Fusb303Transaction));
	xTaskCreate(&fusb303Task, "FUSB303", RESERVE_STACK_USAGE_BYTES(736), NULL, 2, &fusb303TaskHandle);
	fusb303InitialiseI2cModule();
}

static void fusb303InitialiseI2cModule(void)
{
	INTCONCLR = _INTCON_INT1EP_MASK;
	IPC1bits.INT1IP = 2;
	IPC1bits.INT1IS = 0;

	IPC16bits.I2C1MIP = 2;
	IPC16bits.I2C1MIS = 1;
	IFS2CLR = _IFS2_I2C1BCIF_MASK | _IFS2_I2C1MIF_MASK | _IFS2_I2C1SIF_MASK;
	IEC2SET = _IEC2_I2C1MIE_MASK;
	I2C1BRG = BRG_NO_MORE_THAN_400KHZ;
	I2C1CON = _I2C1CON_SDAHT_MASK | _I2C1CON_ON_MASK;
}

void fusb303InitialiseController(void)
{
	vTaskDelay(pdMS_TO_TICKS(100));
	fusb303InitialiseNextRegister((const struct Fusb303Transaction *) 0);
}

static void fusb303InitialiseNextRegister(const struct Fusb303Transaction *transaction)
{
	uint32_t index = 0;
	if (transaction)
		index = ((uint32_t) transaction->context) + (transaction->flags.bits.success ? 1 : 0);

	if (index >= sizeof(fusb303RegisterInitialisation) / sizeof(struct Fusb303Register))
	{
		fusb303OnRegisterInitialisationDone();
		return;
	}

	const struct Fusb303Register *reg = &fusb303RegisterInitialisation[index];
	fusb303TryWriteRegisterWithContext(reg->address, reg->value, &fusb303InitialiseNextRegister, (void *) index);
}

static void fusb303OnRegisterInitialisationDone(void)
{
	IFS0SET = _IFS0_INT1IF_MASK;
	IEC0SET = _IEC0_INT1IE_MASK;
	isInitialised = true;
}

bool fusb303IsInitialised(void)
{
	return isInitialised;
}
