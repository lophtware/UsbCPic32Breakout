#include <xc.h>
#include <stdint.h>

#include "Pins.h"

#include "../FreeRtos.h"

struct Masks
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
};

static void pinsGetMasks(struct Masks *masks, uint16_t mask);

void pinsMaskedLatSet(uint16_t mask)
{
	struct Masks masks;
	pinsGetMasks(&masks, mask);
	taskENTER_CRITICAL();
	LATASET = masks.a;
	LATBSET = masks.b;
	LATCSET = masks.c;
	taskEXIT_CRITICAL();
}

static void pinsGetMasks(struct Masks *masks, uint16_t mask)
{
	if (!masks)
		return;

	masks->a = 0;
	masks->b = 0;
	masks->c = 0;

	for (int i = 0; i < sizeof(pins->pinMaskMap) / sizeof(struct PinMaskMap); i++)
	{
		if (mask & 0x0001)
		{
			switch (pins->pinMaskMap[i].bank)
			{
				case 0:
					masks->a |= pins->pinMaskMap[i].mask;
					break;

				case 1:
					masks->b |= pins->pinMaskMap[i].mask;
					break;

				case 2:
					masks->c |= pins->pinMaskMap[i].mask;
					break;

				default:
					break;
			}
		}

		mask >>= 1;
	}

	masks->a &= pinsAssignedMaskA;
	masks->b &= pinsAssignedMaskB;
	masks->c &= pinsAssignedMaskC;
}

void pinsMaskedLatClear(uint16_t mask)
{
	struct Masks masks;
	pinsGetMasks(&masks, mask);
	taskENTER_CRITICAL();
	LATACLR = masks.a;
	LATBCLR = masks.b;
	LATCCLR = masks.c;
	taskEXIT_CRITICAL();
}

void pinsMaskedLatToggle(uint16_t mask)
{
	struct Masks masks;
	pinsGetMasks(&masks, mask);
	taskENTER_CRITICAL();
	LATAINV = masks.a;
	LATBINV = masks.b;
	LATCINV = masks.c;
	taskEXIT_CRITICAL();
}

void pinsMaskedLatLoad(uint16_t mask)
{
	struct Masks masks;
	pinsGetMasks(&masks, mask);
	taskENTER_CRITICAL();
	LATACLR = pinsAssignedMaskA;
	LATASET = masks.a;
	LATBCLR = pinsAssignedMaskB;
	LATBSET = masks.b;
	LATCCLR = pinsAssignedMaskC;
	LATCSET = masks.c;
	taskEXIT_CRITICAL();
}
