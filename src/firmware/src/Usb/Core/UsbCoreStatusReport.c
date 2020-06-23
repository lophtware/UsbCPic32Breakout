#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "../../Fault.h"
#include "../../Configuration.h"

#include "UsbCoreInterface.h"

struct StatusReport
{
	uint32_t resetReason;
	uint32_t errorEpc;
	uint32_t epc;
	uint32_t cause;
	uint32_t desave;
	uint32_t ra;
	uint64_t debug;
	uint32_t storedCrc32;
	uint32_t calculatedCrc32;
};

int16_t usbCoreGetStatusReport(uint8_t *report)
{
	struct StatusReport status =
	{
		.resetReason = faultGetResetReason(),
		.errorEpc = faultGetExceptionDetailErrorEpc(),
		.epc = faultGetExceptionDetailEpc(),
		.cause = faultGetExceptionDetailCause(),
		.desave = faultGetExceptionDetailDesave(),
		.ra = faultGetExceptionDetailRa(),
		.debug = faultGetDebugDword(),
		.storedCrc32 = (uint32_t) ((configurationGetCrc32s() >> 0) & 0xfffffffful),
		.calculatedCrc32 = (uint32_t) ((configurationGetCrc32s() >> 32) & 0xfffffffful),
	};

	report[0] = CORE_STATUS_REPORT_ID;
	memcpy(report + 1, &status, sizeof(struct StatusReport));
	return 0;
}
