#include "rsrx_platform_adapters.h"

static uint32_t uPortTableIsValid(
	const rsrx_platform_port_table_t * pxPorts)
{
	return (uint32_t)((pxPorts != (const rsrx_platform_port_table_t *)0) &&
		(pxPorts->xClock.pfNow != (rsrx_clock_now_fn)0) &&
		(pxPorts->xTimer.pfCommand != (rsrx_timer_command_fn)0) &&
		(pxPorts->xDiagnostics.pfWrite != (rsrx_diagnostic_write_fn)0));
}

static uint32_t uExecutorIsValid(
	const rsrx_action_executor_t * pxExecutor)
{
	return (uint32_t)((pxExecutor != (const rsrx_action_executor_t *)0) &&
		(pxExecutor->pfDispatch != (rsrx_action_dispatch_fn)0));
}

static rsrx_timer_id_t eMapTimerId(
	rsrx_action_t eAction)
{
	switch(eAction)
	{
		case RSRX_ACTION_START_SUPERVISION_TIMER:
		case RSRX_ACTION_RESET_SUPERVISION_TIMER:
			return RSRX_TIMER_ID_SUPERVISION;

		default:
			return RSRX_TIMER_ID_INVALID;
	}
}

static rsrx_timer_command_type_t eMapTimerCommand(
	rsrx_action_t eAction)
{
	switch(eAction)
	{
		case RSRX_ACTION_START_SUPERVISION_TIMER:
			return RSRX_TIMER_COMMAND_START;

		case RSRX_ACTION_RESET_SUPERVISION_TIMER:
			return RSRX_TIMER_COMMAND_RESTART;

		default:
			return RSRX_TIMER_COMMAND_NONE;
	}
}

static rsrx_monotonic_time_ns_t uResolveInterval(
	const rsrx_platform_adapter_context_t * pxContext,
	rsrx_timer_id_t eTimerId)
{
	switch(eTimerId)
	{
		case RSRX_TIMER_ID_SUPERVISION:
			return pxContext->uSupervisionIntervalNs;

		case RSRX_TIMER_ID_RETRANSMISSION:
			return pxContext->uRetransmissionIntervalNs;

		case RSRX_TIMER_ID_DIAGNOSTIC_FLUSH:
			return pxContext->uDiagnosticFlushIntervalNs;

		case RSRX_TIMER_ID_INVALID:
		default:
			return 0U;
	}
}

static rsrx_log_severity_t eMapSeverity(
	rsrx_diagnostic_code_t eDiagnostic)
{
	switch(eDiagnostic)
	{
		case RSRX_DIAG_INFO_STATE_TRANSITION:
		case RSRX_DIAG_INFO_OPERATIONAL_EVENT:
			return RSRX_LOG_SEVERITY_INFO;

		case RSRX_DIAG_WARN_REJECTED_EVENT:
		case RSRX_DIAG_WARN_IGNORED_EVENT:
			return RSRX_LOG_SEVERITY_WARNING;

		case RSRX_DIAG_ERROR_TIMEOUT:
		case RSRX_DIAG_ERROR_PROTOCOL:
		case RSRX_DIAG_ERROR_CONFIGURATION:
		case RSRX_DIAG_ERROR_INTERFACE:
		case RSRX_DIAG_ERROR_INTERNAL_STATE:
			return RSRX_LOG_SEVERITY_ERROR;

		case RSRX_DIAG_NONE:
		default:
			return RSRX_LOG_SEVERITY_INFO;
	}
}

rsrx_platform_status_t rsrx_platform_adapter_init(
	rsrx_platform_adapter_context_t * pxContext,
	const rsrx_platform_port_table_t * pxPorts,
	rsrx_monotonic_time_ns_t uSupervisionIntervalNs,
	rsrx_monotonic_time_ns_t uRetransmissionIntervalNs,
	rsrx_monotonic_time_ns_t uDiagnosticFlushIntervalNs)
{
	if((pxContext == (rsrx_platform_adapter_context_t *)0) ||
		(uPortTableIsValid(pxPorts) == 0U))
	{
		return RSRX_PLATFORM_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xPlatformPorts = *pxPorts;
	pxContext->uSupervisionIntervalNs = uSupervisionIntervalNs;
	pxContext->uRetransmissionIntervalNs = uRetransmissionIntervalNs;
	pxContext->uDiagnosticFlushIntervalNs = uDiagnosticFlushIntervalNs;
	pxContext->uEventCounter = 0U;

	return RSRX_PLATFORM_STATUS_OK;
}

void rsrx_platform_timer_executor_dispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	rsrx_platform_adapter_context_t * pxContext = (rsrx_platform_adapter_context_t *)pvContext;
	rsrx_monotonic_time_ns_t uNowNs;
	rsrx_timer_command_t xCommand;
	rsrx_timer_id_t eTimerId;
	(void)uActionIndex;

	if((pxContext == (rsrx_platform_adapter_context_t *)0) ||
		(pxTransition == (const rsrx_transition_result_t *)0))
	{
		return;
	}

	eTimerId = eMapTimerId(eAction);
	if((eTimerId == RSRX_TIMER_ID_INVALID) ||
		(pxContext->xPlatformPorts.xClock.pfNow(pxContext->xPlatformPorts.xClock.pvContext, &uNowNs) != RSRX_PLATFORM_STATUS_OK))
	{
		return;
	}

	xCommand.eTimerId = eTimerId;
	xCommand.eCommandType = eMapTimerCommand(eAction);
	xCommand.uDeadlineNs = uNowNs + uResolveInterval(pxContext, eTimerId);
	xCommand.eReason = pxTransition->eReason;

	(void)pxContext->xPlatformPorts.xTimer.pfCommand(
		pxContext->xPlatformPorts.xTimer.pvContext,
		&xCommand);
}

void rsrx_platform_diagnostics_executor_dispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	rsrx_platform_adapter_context_t * pxContext = (rsrx_platform_adapter_context_t *)pvContext;
	rsrx_diagnostic_record_t xRecord;
	(void)eAction;
	(void)uActionIndex;

	if((pxContext == (rsrx_platform_adapter_context_t *)0) ||
		(pxTransition == (const rsrx_transition_result_t *)0))
	{
		return;
	}

	pxContext->uEventCounter++;

	xRecord.eSeverity = eMapSeverity(pxTransition->eDiagnostic);
	xRecord.ePreviousState = pxTransition->ePreviousState;
	xRecord.eNextState = pxTransition->eNextState;
	xRecord.eStatus = pxTransition->eStatus;
	xRecord.eReason = pxTransition->eReason;
	xRecord.eDiagnostic = pxTransition->eDiagnostic;
	xRecord.uEventCounter = pxContext->uEventCounter;

	(void)pxContext->xPlatformPorts.xDiagnostics.pfWrite(
		pxContext->xPlatformPorts.xDiagnostics.pvContext,
		&xRecord);
}

rsrx_status_t rsrx_platform_adapter_build_executor_table(
	rsrx_action_executor_table_t * pxExecutors,
	rsrx_platform_adapter_context_t * pxPlatformContext,
	const rsrx_action_executor_t * pxTransportExecutor,
	const rsrx_action_executor_t * pxApiExecutor,
	const rsrx_action_executor_t * pxLifecycleExecutor)
{
	if((pxExecutors == (rsrx_action_executor_table_t *)0) ||
		(pxPlatformContext == (rsrx_platform_adapter_context_t *)0) ||
		(uPortTableIsValid(&pxPlatformContext->xPlatformPorts) == 0U) ||
		(uExecutorIsValid(pxTransportExecutor) == 0U) ||
		(uExecutorIsValid(pxApiExecutor) == 0U) ||
		(uExecutorIsValid(pxLifecycleExecutor) == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxExecutors->xTransportExecutor = *pxTransportExecutor;
	pxExecutors->xTimerExecutor.pvContext = pxPlatformContext;
	pxExecutors->xTimerExecutor.pfDispatch = rsrx_platform_timer_executor_dispatch;
	pxExecutors->xApiExecutor = *pxApiExecutor;
	pxExecutors->xDiagnosticsExecutor.pvContext = pxPlatformContext;
	pxExecutors->xDiagnosticsExecutor.pfDispatch = rsrx_platform_diagnostics_executor_dispatch;
	pxExecutors->xLifecycleExecutor = *pxLifecycleExecutor;

	return RSRX_STATUS_OK;
}
