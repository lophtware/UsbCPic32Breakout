#ifndef __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_FUSB303_FUSB303_H
#define __LOPHTWARE_USBCPIC32BREAKOUT_FIRMWARE_USBCC_FUSB303_FUSB303_H
#include <stdint.h>
#include <stdbool.h>
#include "../../FreeRtos.h"
#include "../Fusb303.h"

#define BIT(n) (1 << n)

#define FUSB303_I2C_ADDRESS 0x42
#define FUSB303_I2C_READ 1
#define FUSB303_I2C_WRITE 0

#define FUSB303_REG_PORTROLE 0x03
#define FUSB303_REG_PORTROLE_SNK BIT(1)
#define FUSB303_REG_PORTROLE_ORIENTDEB BIT(6)

#define FUSB303_REG_CONTROL 0x04
#define FUSB303_REG_CONTROL_DCABLE_EN BIT(3)
#define FUSB303_REG_CONTROL_HOSTCUR_DEFAULT BIT(1)

#define FUSB303_REG_CONTROL1 0x05
#define FUSB303_REG_CONTROL1_REMEDY_EN BIT(7)
#define FUSB303_REG_CONTROL1_ENABLE BIT(3)

#define FUSB303_REG_MANUAL 0x09
#define FUSB303_REG_MANUAL_UNATT_SNK BIT(3)

#define FUSB303_REG_RESET 0x0a

#define FUSB303_REG_MASK 0x0e
#define FUSB303_REG_MASK_AUTOSNK BIT(3)

#define FUSB303_REG_MASK1 0x0f
#define FUSB303_REG_MASK1_VBOFF BIT(6)
#define FUSB303_REG_MASK1_VBON BIT(5)
#define FUSB303_REG_MASK1_FRC_FAIL BIT(2)
#define FUSB303_REG_MASK1_FRC_SUCC BIT(1)

#define FUSB303_REG_STATUS 0x11
#define FUSB303_REG_STATUS_ATTACH BIT(0)
#define FUSB303_REG_STATUS_VBUSOK BIT(3)

#define FUSB303_REG_STATUS1 0x12
#define FUSB303_REG_STATUS1_FAULT BIT(1)
#define FUSB303_REG_STATUS1_REMEDY BIT(0)

#define FUSB303_REG_TYPE 0x13
#define FUSB303_REG_TYPE_DEBUGSNK BIT(5)
#define FUSB303_REG_TYPE_ACTIVECABLE BIT(2)
#define FUSB303_REG_TYPE_AUDIOVBUS BIT(1)
#define FUSB303_REG_TYPE_AUDIO BIT(0)

#define FUSB303_REG_INTERRUPT 0x14
#define FUSB303_REG_INTERRUPT_CLEARALL 0x7f

#define FUSB303_REG_INTERRUPT1 0x15
#define FUSB303_REG_INTERRUPT1_CLEARALL 0x7f

struct Fusb303Transaction;
typedef void (*Fusb303TransactionOnDone)(const struct Fusb303Transaction *transaction);

struct Fusb303Register
{
	uint8_t address;
	uint8_t value;
};

struct Fusb303Transaction
{
	uint8_t deviceAddress;
	struct Fusb303Register reg;
	union
	{
		struct
		{
			unsigned int startBitSeen : 1;
			unsigned int isDeviceAddressAckd : 1;
			unsigned int isRegisterAddressAckd : 1;
			unsigned int success : 1;
			unsigned int : 4;
		} bits;

		uint8_t all;
	} flags;
	Fusb303TransactionOnDone onDone;
	void *context;
};

extern QueueHandle_t fusb303Events;
extern QueueHandle_t fusb303Transactions;
extern TaskHandle_t fusb303TaskHandle;

extern void fusb303InitialiseController(void);
extern void fusb303Task(void *args);
extern void fusb303ReadFlagsAndBroadcastFromIsr(BaseType_t *wasHigherPriorityTaskWoken);
extern bool fusb303TryReadRegister(uint8_t registerAddress, Fusb303TransactionOnDone onDone);
extern bool fusb303TryReadRegisterWithContext(uint8_t registerAddress, Fusb303TransactionOnDone onDone, void *context);
extern bool fusb303TryWriteRegister(uint8_t registerAddress, uint8_t value, Fusb303TransactionOnDone onDone);
extern bool fusb303TryWriteRegisterWithContext(uint8_t registerAddress, uint8_t value, Fusb303TransactionOnDone onDone, void *context);

#endif
