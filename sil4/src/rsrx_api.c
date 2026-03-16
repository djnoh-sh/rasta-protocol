#include "rsrx_api.h"
#include "rsrx_config_validator.h"

static void vApiExecutorDispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	rsrx_session_t * pxSession = (rsrx_session_t *)pvContext;
	(void)pxTransition;
	(void)eAction;
	(void)uActionIndex;

	if((pxSession != (rsrx_session_t *)0) &&
		(pxSession->pfApiNotification != (rsrx_api_notification_fn)0))
	{
		pxSession->pfApiNotification(
			pxSession->pvApiCallbackContext,
			&pxSession->xLastReport);
	}
}

static void vLifecycleExecutorDispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	rsrx_session_t * pxSession = (rsrx_session_t *)pvContext;
	(void)pxTransition;

	if((pxSession != (rsrx_session_t *)0) &&
		(pxSession->pfLifecycleNotification != (rsrx_lifecycle_notification_fn)0))
	{
		pxSession->pfLifecycleNotification(
			pxSession->pvLifecycleCallbackContext,
			&pxSession->xLastReport,
			eAction,
			uActionIndex);
	}
}

static rsrx_status_t eProcessSessionEvent(
	rsrx_session_t * pxSession,
	rsrx_event_t eEvent,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	rsrx_status_t eStatus;

	if((pxSession == (rsrx_session_t *)0) ||
		(ppxReport == (const rsrx_orchestrator_report_t **)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	eStatus = rsrx_orchestrator_process_event(
		&pxSession->xOrchestrator,
		eEvent,
		&pxSession->xLastReport);

	*ppxReport = &pxSession->xLastReport;

	return eStatus;
}

static rsrx_status_t eMapTimerSourceToEvent(
	rsrx_timer_expiry_source_t eTimerSource,
	rsrx_event_t * peEvent)
{
	if(peEvent == (rsrx_event_t *)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	switch(eTimerSource)
	{
		case RSRX_TIMER_EXPIRY_SUPERVISION:
			*peEvent = RSRX_EVENT_TIMEOUT;
			return RSRX_STATUS_OK;

		case RSRX_TIMER_EXPIRY_RETRANSMISSION:
			*peEvent = RSRX_EVENT_RETRANSMISSION_FAILURE;
			return RSRX_STATUS_OK;

		case RSRX_TIMER_EXPIRY_DIAGNOSTIC_FLUSH:
		case RSRX_TIMER_EXPIRY_INVALID:
		default:
			return RSRX_STATUS_INVALID_ARGUMENT;
	}
}

rsrx_status_t rsrx_session_init(
	rsrx_session_t * pxSession,
	const rsrx_session_config_t * pxConfig)
{
	rsrx_action_executor_t xApiExecutor;
	rsrx_action_executor_t xLifecycleExecutor;
	rsrx_config_validation_report_t xValidationReport;

	if((pxSession == (rsrx_session_t *)0) ||
		(rsrx_validate_session_config(pxConfig, &xValidationReport) != RSRX_CONFIG_STATUS_OK))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(rsrx_transport_adapter_init(
		&pxSession->xTransportAdapter,
		&pxConfig->xTransportPort,
		&pxConfig->xCodecPort,
		pxConfig->eDefaultChannelId,
		pxConfig->puFramePayload,
		pxConfig->xFramePayloadLength) != RSRX_TRANSPORT_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(rsrx_platform_adapter_init(
		&pxSession->xPlatformAdapter,
		&pxConfig->xPlatformPorts,
		pxConfig->uSupervisionIntervalNs,
		pxConfig->uRetransmissionIntervalNs,
		pxConfig->uDiagnosticFlushIntervalNs) != RSRX_PLATFORM_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxSession->pvApiCallbackContext = pxConfig->pvApiCallbackContext;
	pxSession->pfApiNotification = pxConfig->pfApiNotification;
	pxSession->pvLifecycleCallbackContext = pxConfig->pvLifecycleCallbackContext;
	pxSession->pfLifecycleNotification = pxConfig->pfLifecycleNotification;

	xApiExecutor.pvContext = pxSession;
	xApiExecutor.pfDispatch = vApiExecutorDispatch;
	xLifecycleExecutor.pvContext = pxSession;
	xLifecycleExecutor.pfDispatch = vLifecycleExecutorDispatch;

	if(rsrx_platform_adapter_build_executor_table(
		&pxSession->xExecutors,
		&pxSession->xTransportAdapter,
		&pxSession->xPlatformAdapter,
		&xApiExecutor,
		&xLifecycleExecutor) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(rsrx_orchestrator_init(&pxSession->xOrchestrator, &pxSession->xExecutors) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxSession->uInitialized = 1U;
	pxSession->xLastReport.uDispatchedActionCount = 0U;
	pxSession->xLastReport.xTransition.ePreviousState = RSRX_STATE_INVALID;
	pxSession->xLastReport.xTransition.eNextState = RSRX_STATE_INVALID;
	pxSession->xLastReport.xTransition.eStatus = RSRX_STATUS_OK;
	pxSession->xLastReport.xTransition.eReason = RSRX_REASON_NONE;
	pxSession->xLastReport.xTransition.eDiagnostic = RSRX_DIAG_NONE;
	pxSession->xLastReport.xTransition.xActions.uActionCount = 0U;

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_session_start(
	rsrx_session_t * pxSession,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	return eProcessSessionEvent(pxSession, RSRX_EVENT_INIT_SUCCESS, ppxReport);
}

rsrx_status_t rsrx_session_connect(
	rsrx_session_t * pxSession,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	return eProcessSessionEvent(pxSession, RSRX_EVENT_CONNECT_REQUEST, ppxReport);
}

rsrx_status_t rsrx_session_disconnect(
	rsrx_session_t * pxSession,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	return eProcessSessionEvent(pxSession, RSRX_EVENT_DISCONNECT_REQUEST, ppxReport);
}

rsrx_status_t rsrx_session_process_event(
	rsrx_session_t * pxSession,
	rsrx_event_t eEvent,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	return eProcessSessionEvent(pxSession, eEvent, ppxReport);
}

rsrx_status_t rsrx_session_process_timer_expiry(
	rsrx_session_t * pxSession,
	rsrx_timer_expiry_source_t eTimerSource,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	rsrx_event_t eEvent;

	if(eMapTimerSourceToEvent(eTimerSource, &eEvent) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return eProcessSessionEvent(pxSession, eEvent, ppxReport);
}

rsrx_state_t rsrx_session_get_state(
	const rsrx_session_t * pxSession)
{
	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATE_INVALID;
	}

	return rsrx_orchestrator_get_state(&pxSession->xOrchestrator);
}

rsrx_status_t rsrx_session_reset(
	rsrx_session_t * pxSession)
{
	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return rsrx_orchestrator_reset(&pxSession->xOrchestrator);
}
