#include "rsrx_orchestrator.h"

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
	const rsrx_action_sink_t * pxActionSink)
{
	if((pxContext == (rsrx_orchestrator_context_t *)0) ||
		(pxActionSink == (const rsrx_action_sink_t *)0) ||
		(pxActionSink->pfDispatch == (rsrx_action_dispatch_fn)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xActionSink = *pxActionSink;

	return rsrx_state_machine_init(&pxContext->xStateMachine);
}

rsrx_status_t rsrx_orchestrator_process_event(
	rsrx_orchestrator_context_t * pxContext,
	rsrx_event_t eEvent,
	rsrx_orchestrator_report_t * pxReport)
{
	rsrx_status_t eStatus;
	uint32_t uIndex;

	if((pxContext == (rsrx_orchestrator_context_t *)0) ||
		(pxReport == (rsrx_orchestrator_report_t *)0) ||
		(pxContext->xActionSink.pfDispatch == (rsrx_action_dispatch_fn)0))
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
		pxContext->xActionSink.pfDispatch(
			pxContext->xActionSink.pvContext,
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
