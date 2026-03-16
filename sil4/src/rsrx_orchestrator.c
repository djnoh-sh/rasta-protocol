#include "rsrx_orchestrator.h"

static uint32_t uExecutorIsValid(
	const rsrx_action_executor_t * pxExecutor)
{
	return (uint32_t)((pxExecutor != (const rsrx_action_executor_t *)0) &&
		(pxExecutor->pfDispatch != (rsrx_action_dispatch_fn)0));
}

static uint32_t uExecutorTableIsValid(
	const rsrx_action_executor_table_t * pxExecutors)
{
	return (uint32_t)((pxExecutors != (const rsrx_action_executor_table_t *)0) &&
		(uExecutorIsValid(&pxExecutors->xTransportExecutor) != 0U) &&
		(uExecutorIsValid(&pxExecutors->xTimerExecutor) != 0U) &&
		(uExecutorIsValid(&pxExecutors->xApplicationExecutor) != 0U) &&
		(uExecutorIsValid(&pxExecutors->xApiExecutor) != 0U) &&
		(uExecutorIsValid(&pxExecutors->xDiagnosticsExecutor) != 0U) &&
		(uExecutorIsValid(&pxExecutors->xLifecycleExecutor) != 0U));
}

static const rsrx_action_executor_t * pxSelectExecutor(
	const rsrx_orchestrator_context_t * pxContext,
	rsrx_action_t eAction)
{
	switch(eAction)
	{
		case RSRX_ACTION_START_HANDSHAKE:
		case RSRX_ACTION_ACCEPT_INBOUND_CONNECT:
		case RSRX_ACTION_SEND_HEARTBEAT:
		case RSRX_ACTION_REQUEST_RETRANSMISSION:
		case RSRX_ACTION_SEND_DISCONNECT:
			return &pxContext->xExecutors.xTransportExecutor;

		case RSRX_ACTION_DELIVER_DATA:
			return &pxContext->xExecutors.xApplicationExecutor;

		case RSRX_ACTION_START_SUPERVISION_TIMER:
		case RSRX_ACTION_RESET_SUPERVISION_TIMER:
			return &pxContext->xExecutors.xTimerExecutor;

		case RSRX_ACTION_NOTIFY_API:
			return &pxContext->xExecutors.xApiExecutor;

		case RSRX_ACTION_LOG_DIAGNOSTIC:
			return &pxContext->xExecutors.xDiagnosticsExecutor;

		case RSRX_ACTION_CLEAR_RETRANSMISSION_CONTEXT:
		case RSRX_ACTION_ENTER_FAILSAFE:
		case RSRX_ACTION_RELEASE_CONNECTION_RESOURCES:
		case RSRX_ACTION_FINALIZE_SHUTDOWN:
			return &pxContext->xExecutors.xLifecycleExecutor;

		case RSRX_ACTION_NONE:
		default:
			return (const rsrx_action_executor_t *)0;
	}
}

static void vResetReport(
	rsrx_orchestrator_report_t * pxReport)
{
	uint32_t uIndex;

	pxReport->xTransition.ePreviousState = RSRX_STATE_INVALID;
	pxReport->xTransition.eNextState = RSRX_STATE_INVALID;
	pxReport->xTransition.eStatus = RSRX_STATUS_INVALID_ARGUMENT;
	pxReport->xTransition.eReason = RSRX_REASON_INVALID_INPUT_ARGUMENT;
	pxReport->xTransition.eDiagnostic = RSRX_DIAG_ERROR_INTERFACE;
	pxReport->xTransition.xActions.uActionCount = 0U;
	pxReport->uDispatchedActionCount = 0U;

	for(uIndex = 0U; uIndex < D_RSRX_ACTION_CAPACITY; ++uIndex)
	{
		pxReport->xTransition.xActions.eActions[uIndex] = RSRX_ACTION_NONE;
	}
}

rsrx_status_t rsrx_orchestrator_init(
	rsrx_orchestrator_context_t * pxContext,
	const rsrx_action_executor_table_t * pxExecutors)
{
	if((pxContext == (rsrx_orchestrator_context_t *)0) ||
		(uExecutorTableIsValid(pxExecutors) == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xExecutors = *pxExecutors;

	return rsrx_state_machine_init(&pxContext->xStateMachine);
}

rsrx_status_t rsrx_orchestrator_process_event(
	rsrx_orchestrator_context_t * pxContext,
	rsrx_event_t eEvent,
	rsrx_orchestrator_report_t * pxReport)
{
	rsrx_status_t eStatus;
	uint32_t uIndex;
	const rsrx_action_executor_t * pxExecutor;

	if((pxContext == (rsrx_orchestrator_context_t *)0) ||
		(pxReport == (rsrx_orchestrator_report_t *)0) ||
		(uExecutorTableIsValid(&pxContext->xExecutors) == 0U))
	{
		if(pxReport != (rsrx_orchestrator_report_t *)0)
		{
			vResetReport(pxReport);
		}

		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	eStatus = rsrx_state_machine_handle_event(
		&pxContext->xStateMachine,
		eEvent,
		&pxReport->xTransition);

	pxReport->uDispatchedActionCount = 0U;

	for(uIndex = 0U; uIndex < pxReport->xTransition.xActions.uActionCount; ++uIndex)
	{
		pxExecutor = pxSelectExecutor(
			pxContext,
			pxReport->xTransition.xActions.eActions[uIndex]);

		if(pxExecutor == (const rsrx_action_executor_t *)0)
		{
			vResetReport(pxReport);
			return RSRX_STATUS_INVALID_STATE;
		}

		pxExecutor->pfDispatch(
			pxExecutor->pvContext,
			&pxReport->xTransition,
			pxReport->xTransition.xActions.eActions[uIndex],
			uIndex);
		pxReport->uDispatchedActionCount++;
	}

	return eStatus;
}

rsrx_state_t rsrx_orchestrator_get_state(
	const rsrx_orchestrator_context_t * pxContext)
{
	if(pxContext == (const rsrx_orchestrator_context_t *)0)
	{
		return RSRX_STATE_INVALID;
	}

	return rsrx_state_machine_get_state(&pxContext->xStateMachine);
}

rsrx_status_t rsrx_orchestrator_reset(
	rsrx_orchestrator_context_t * pxContext)
{
	if(pxContext == (rsrx_orchestrator_context_t *)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return rsrx_state_machine_reset(&pxContext->xStateMachine);
}
