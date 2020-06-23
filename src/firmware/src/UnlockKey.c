#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "UnlockKey.h"

static struct UnlockKey unlockKey;

void unlockKeySet(const struct UnlockKey *key)
{
	if (!key)
		return;

	unlockKey.as.dwords[0] = key->as.dwords[0];
	unlockKey.as.dwords[1] = key->as.dwords[1];
}

bool unlockKeyMatches(const struct UnlockKey *key)
{
	return key && key->as.dwords[0] == unlockKey.as.dwords[0] && key->as.dwords[1] == unlockKey.as.dwords[1];
}
