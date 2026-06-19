#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>

#include "posix_platform.h"

static rsrx_platform_status_t ePosixClockNow(
	void * pvContext,
	rsrx_monotonic_time_ns_t * puNowNs)
{
	struct timespec xTimestamp;

	(void)pvContext;
	if(puNowNs == (rsrx_monotonic_time_ns_t *)0)
	{
		return RSRX_PLATFORM_STATUS_INVALID_ARGUMENT;
	}

	if(clock_gettime(CLOCK_MONOTONIC, &xTimestamp) != 0)
	{
		return RSRX_PLATFORM_STATUS_INTERNAL_ERROR;
	}

	*puNowNs = ((rsrx_monotonic_time_ns_t)xTimestamp.tv_sec * 1000000000ULL) +
		(rsrx_monotonic_time_ns_t)xTimestamp.tv_nsec;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t ePosixTimerCommand(
	void * pvContext,
	const rsrx_timer_command_t * pxCommand)
{
	posix_timer_context_t * pxContext = (posix_timer_context_t *)pvContext;
	uint32_t uActive;

	if((pxContext == (posix_timer_context_t *)0) ||
		(pxCommand == (const rsrx_timer_command_t *)0))
	{
		return RSRX_PLATFORM_STATUS_INVALID_ARGUMENT;
	}

	uActive = (uint32_t)((pxCommand->eCommandType == RSRX_TIMER_COMMAND_START) ||
		(pxCommand->eCommandType == RSRX_TIMER_COMMAND_RESTART));

	switch(pxCommand->eTimerId)
	{
		case RSRX_TIMER_ID_SUPERVISION:
			pxContext->uSupervisionActive = uActive;
			if(uActive != 0U)
			{
				pxContext->uSupervisionDeadline = pxCommand->uDeadlineNs;
			}
			break;

		case RSRX_TIMER_ID_RETRANSMISSION:
			pxContext->uRetransmissionActive = uActive;
			if(uActive != 0U)
			{
				pxContext->uRetransmissionDeadline = pxCommand->uDeadlineNs;
			}
			break;

		case RSRX_TIMER_ID_DIAGNOSTIC_FLUSH:
			pxContext->uDiagnosticFlushActive = uActive;
			if(uActive != 0U)
			{
				pxContext->uDiagnosticFlushDeadline = pxCommand->uDeadlineNs;
			}
			break;

		case RSRX_TIMER_ID_INVALID:
		default:
			break;
	}

	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t ePosixDiagnosticWrite(
	void * pvContext,
	const rsrx_diagnostic_record_t * pxRecord)
{
	const char * pcSeverity;
	struct timespec xTimestamp;
	long long iMilliseconds;

	(void)pvContext;
	if(pxRecord == (const rsrx_diagnostic_record_t *)0)
	{
		return RSRX_PLATFORM_STATUS_INVALID_ARGUMENT;
	}

	pcSeverity = "INFO";
	if(pxRecord->eSeverity == RSRX_LOG_SEVERITY_WARNING)
	{
		pcSeverity = "WARN";
	}
	else if(pxRecord->eSeverity == RSRX_LOG_SEVERITY_ERROR)
	{
		pcSeverity = "ERROR";
	}
	else if(pxRecord->eSeverity == RSRX_LOG_SEVERITY_FATAL)
	{
		pcSeverity = "FATAL";
	}

	(void)clock_gettime(CLOCK_MONOTONIC, &xTimestamp);
	iMilliseconds = ((long long)xTimestamp.tv_sec * 1000LL) +
		((long long)xTimestamp.tv_nsec / 1000000LL);
	(void)printf(
		"[%lld ms] [%s] diag=0x%04X state=%u->%u status=%u reason=0x%04X event_count=%u\n",
		iMilliseconds,
		pcSeverity,
		pxRecord->eDiagnostic,
		(unsigned int)pxRecord->ePreviousState,
		(unsigned int)pxRecord->eNextState,
		(unsigned int)pxRecord->eStatus,
		(unsigned int)pxRecord->eReason,
		(unsigned int)pxRecord->uEventCounter);
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t ePosixCriticalSectionEnter(
	void * pvContext)
{
	(void)pvContext;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t ePosixCriticalSectionExit(
	void * pvContext)
{
	(void)pvContext;
	return RSRX_PLATFORM_STATUS_OK;
}

void posix_platform_init(
	rsrx_platform_port_table_t * pxPlatformTable,
	posix_timer_context_t * pxTimerCtx)
{
	if((pxPlatformTable == (rsrx_platform_port_table_t *)0) ||
		(pxTimerCtx == (posix_timer_context_t *)0))
	{
		return;
	}

	pxTimerCtx->uSupervisionActive = 0U;
	pxTimerCtx->uRetransmissionActive = 0U;
	pxTimerCtx->uDiagnosticFlushActive = 0U;
	pxTimerCtx->uSupervisionDeadline = 0U;
	pxTimerCtx->uRetransmissionDeadline = 0U;
	pxTimerCtx->uDiagnosticFlushDeadline = 0U;

	pxPlatformTable->xClock.pvContext = (void *)0;
	pxPlatformTable->xClock.pfNow = ePosixClockNow;

	pxPlatformTable->xTimer.pvContext = pxTimerCtx;
	pxPlatformTable->xTimer.pfCommand = ePosixTimerCommand;

	pxPlatformTable->xDiagnostics.pvContext = (void *)0;
	pxPlatformTable->xDiagnostics.pfWrite = ePosixDiagnosticWrite;

	pxPlatformTable->xCriticalSection.pvContext = (void *)0;
	pxPlatformTable->xCriticalSection.pfEnter = ePosixCriticalSectionEnter;
	pxPlatformTable->xCriticalSection.pfExit = ePosixCriticalSectionExit;
}
