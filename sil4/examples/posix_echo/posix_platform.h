#ifndef POSIX_PLATFORM_H
#define POSIX_PLATFORM_H

#include "rsrx_platform.h"

typedef struct
{
	rsrx_monotonic_time_ns_t uSupervisionDeadline;
	rsrx_monotonic_time_ns_t uRetransmissionDeadline;
	rsrx_monotonic_time_ns_t uDiagnosticFlushDeadline;
	uint32_t uSupervisionActive;
	uint32_t uRetransmissionActive;
	uint32_t uDiagnosticFlushActive;
} posix_timer_context_t;

void posix_platform_init(
	rsrx_platform_port_table_t * pxPlatformTable,
	posix_timer_context_t * pxTimerCtx);

#endif
