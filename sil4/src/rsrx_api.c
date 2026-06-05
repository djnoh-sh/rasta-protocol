#include "rsrx_api.h"
#include "rsrx_config_validator.h"

static void vSetDirectReport(
	rsrx_session_t * pxSession,
	rsrx_status_t eStatus,
	rsrx_reason_code_t eReason,
	rsrx_diagnostic_code_t eDiagnostic)
{
	rsrx_state_t eCurrentState;
	uint32_t uIndex;

	if(pxSession == (rsrx_session_t *)0)
	{
		return;
	}

	eCurrentState = rsrx_orchestrator_get_state(&pxSession->xOrchestrator);
	pxSession->xLastReport.xTransition.ePreviousState = eCurrentState;
	pxSession->xLastReport.xTransition.eNextState = eCurrentState;
	pxSession->xLastReport.xTransition.eStatus = eStatus;
	pxSession->xLastReport.xTransition.eReason = eReason;
	pxSession->xLastReport.xTransition.eDiagnostic = eDiagnostic;
	pxSession->xLastReport.xTransition.xActions.uActionCount = 0U;
	for(uIndex = 0U; uIndex < D_RSRX_ACTION_CAPACITY; ++uIndex)
	{
		pxSession->xLastReport.xTransition.xActions.eActions[uIndex] = RSRX_ACTION_NONE;
	}
	pxSession->xLastReport.uDispatchedActionCount = 0U;
}

static void vResetLastReport(
	rsrx_session_t * pxSession)
{
	uint32_t uIndex;

	if(pxSession == (rsrx_session_t *)0)
	{
		return;
	}

	pxSession->xLastReport.uDispatchedActionCount = 0U;
	pxSession->xLastReport.xTransition.ePreviousState = RSRX_STATE_INVALID;
	pxSession->xLastReport.xTransition.eNextState = RSRX_STATE_INVALID;
	pxSession->xLastReport.xTransition.eStatus = RSRX_STATUS_OK;
	pxSession->xLastReport.xTransition.eReason = RSRX_REASON_NONE;
	pxSession->xLastReport.xTransition.eDiagnostic = RSRX_DIAG_NONE;
	pxSession->xLastReport.xTransition.xActions.uActionCount = 0U;
	for(uIndex = 0U; uIndex < D_RSRX_ACTION_CAPACITY; ++uIndex)
	{
		pxSession->xLastReport.xTransition.xActions.eActions[uIndex] = RSRX_ACTION_NONE;
	}
}

static void vClearOutboundTelemetry(
	rsrx_outbound_send_telemetry_t * pxTelemetry)
{
	if(pxTelemetry == (rsrx_outbound_send_telemetry_t *)0)
	{
		return;
	}

	pxTelemetry->eLastSendStatus = RSRX_TRANSPORT_STATUS_OK;
	pxTelemetry->eLastRejectReason = RSRX_OUTBOUND_REJECT_REASON_NONE;
	pxTelemetry->uAcceptedSendCount = 0U;
	pxTelemetry->uQueuedSendCount = 0U;
	pxTelemetry->uMaxDeferredSendCount = 0U;
	pxTelemetry->uDeferredDispatchCount = 0U;
	pxTelemetry->uQueueOverflowRejectCount = 0U;
	pxTelemetry->uBusyRejectedSendCount = 0U;
	pxTelemetry->uConsecutiveBusyRejectedSendCount = 0U;
	pxTelemetry->uMaxConsecutiveBusyRejectedSendCount = 0U;
	pxTelemetry->uBusyRejectEscalationCount = 0U;
	pxTelemetry->uLastBusyRejectEscalated = 0U;
	pxTelemetry->uClearOnInboundCount = 0U;
	pxTelemetry->uClearOnFeedbackCount = 0U;
	pxTelemetry->uClearManualCount = 0U;
	pxTelemetry->uRuntimeResetCount = 0U;
}

static void vClearOutboundQueueSnapshot(
	rsrx_outbound_queue_snapshot_t * pxSnapshot)
{
	if(pxSnapshot == (rsrx_outbound_queue_snapshot_t *)0)
	{
		return;
	}

	pxSnapshot->uOutstandingSendPresent = 0U;
	pxSnapshot->eOutstandingSendChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxSnapshot->uDeferredSendPresent = 0U;
	pxSnapshot->uDeferredSendCount = 0U;
	vClearOutboundTelemetry(&pxSnapshot->xTelemetry);
}

static void vClearChannelManagerSnapshot(
	rsrx_channel_manager_snapshot_t * pxSnapshot)
{
	if(pxSnapshot == (rsrx_channel_manager_snapshot_t *)0)
	{
		return;
	}

	pxSnapshot->eActiveChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxSnapshot->ePreferredChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxSnapshot->uAvailableChannelCount = 0U;
	pxSnapshot->uLastSelectionWasFailover = 0U;
	pxSnapshot->uPreferredRecoveryStableSelectionCount = 0U;
	pxSnapshot->uPreferredRecoveryPendingPenaltySelections = 0U;
	pxSnapshot->uPreferredRecoveryPenaltyArmCount = 0U;
	pxSnapshot->uPreferredRecoveryPenaltyRearmCount = 0U;
	pxSnapshot->uPreferredRecoveryPenaltyAppliedCycleCount = 0U;
	pxSnapshot->uPreferredRecoveryPenaltyAbortCount = 0U;
	pxSnapshot->uPreferredRecoveryPenaltyClearCount = 0U;
	pxSnapshot->uPreferredRecoveryPenaltyBypassClearCount = 0U;
	pxSnapshot->uPreferredRecoveryPenaltyResetClearCount = 0U;
	pxSnapshot->uPreferredRecoveryHoldoffTargetCount = 0U;
	pxSnapshot->uPreferredRecoveryHoldoffRemainingCount = 0U;
	pxSnapshot->uTotalSwitchCount = 0U;
	pxSnapshot->uUnavailableSelectionCount = 0U;
}

static uint32_t uGetChannelManagerEffectiveHoldoffTarget(
	const rsrx_channel_manager_context_t * pxChannelManager)
{
	uint32_t uTarget;

	uTarget = pxChannelManager->xConfig.uPreferredRecoveryHoldoffSelections;
	if((UINT32_MAX - uTarget) <
		pxChannelManager->uPreferredRecoveryPendingPenaltySelections)
	{
		return UINT32_MAX;
	}

	return uTarget +
		pxChannelManager->uPreferredRecoveryPendingPenaltySelections;
}

static uint32_t uCountChannelManagerAvailableChannels(
	const rsrx_channel_manager_context_t * pxChannelManager)
{
	uint32_t uIndex;
	uint32_t uCount;

	uCount = 0U;
	for(uIndex = 0U; uIndex < pxChannelManager->xConfig.uChannelCount; ++uIndex)
	{
		if(pxChannelManager->xConfig.axChannels[uIndex].uIsAvailable != 0U)
		{
			uCount++;
		}
	}

	return uCount;
}

static rsrx_diagnostic_code_t eResolveBusyRejectDiagnostic(
	const rsrx_session_t * pxSession)
{
	const rsrx_outbound_send_telemetry_t * pxTelemetry;

	if(pxSession == (const rsrx_session_t *)0)
	{
		return RSRX_DIAG_WARN_REJECTED_EVENT;
	}

	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(
		&pxSession->xTransportAdapter);
	if((pxTelemetry != (const rsrx_outbound_send_telemetry_t *)0) &&
		(pxSession->uBusyRejectErrorThreshold > 0U) &&
		(pxTelemetry->uConsecutiveBusyRejectedSendCount >=
			pxSession->uBusyRejectErrorThreshold))
	{
		return RSRX_DIAG_ERROR_INTERFACE;
	}

	return RSRX_DIAG_WARN_REJECTED_EVENT;
}

static rsrx_log_severity_t eMapDirectDiagnosticSeverity(
	rsrx_diagnostic_code_t eDiagnostic)
{
	switch(eDiagnostic)
	{
		case RSRX_DIAG_ERROR_TIMEOUT:
		case RSRX_DIAG_ERROR_PROTOCOL:
		case RSRX_DIAG_ERROR_CONFIGURATION:
		case RSRX_DIAG_ERROR_INTERFACE:
		case RSRX_DIAG_ERROR_INTERNAL_STATE:
			return RSRX_LOG_SEVERITY_ERROR;

		case RSRX_DIAG_WARN_REJECTED_EVENT:
		case RSRX_DIAG_WARN_IGNORED_EVENT:
			return RSRX_LOG_SEVERITY_WARNING;

		case RSRX_DIAG_INFO_STATE_TRANSITION:
		case RSRX_DIAG_INFO_OPERATIONAL_EVENT:
		case RSRX_DIAG_NONE:
		default:
			return RSRX_LOG_SEVERITY_INFO;
	}
}

static void vWriteDirectDiagnostic(
	rsrx_session_t * pxSession)
{
	rsrx_diagnostic_record_t xRecord;

	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->xPlatformAdapter.xPlatformPorts.xDiagnostics.pfWrite ==
			(rsrx_diagnostic_write_fn)0))
	{
		return;
	}

	pxSession->xPlatformAdapter.uEventCounter++;
	xRecord.eSeverity = eMapDirectDiagnosticSeverity(
		pxSession->xLastReport.xTransition.eDiagnostic);
	xRecord.ePreviousState = pxSession->xLastReport.xTransition.ePreviousState;
	xRecord.eNextState = pxSession->xLastReport.xTransition.eNextState;
	xRecord.eStatus = pxSession->xLastReport.xTransition.eStatus;
	xRecord.eReason = pxSession->xLastReport.xTransition.eReason;
	xRecord.eDiagnostic = pxSession->xLastReport.xTransition.eDiagnostic;
	xRecord.uEventCounter = pxSession->xPlatformAdapter.uEventCounter;

	(void)pxSession->xPlatformAdapter.xPlatformPorts.xDiagnostics.pfWrite(
		pxSession->xPlatformAdapter.xPlatformPorts.xDiagnostics.pvContext,
		&xRecord);
}

static rsrx_status_t eCancelSessionTimer(
	rsrx_session_t * pxSession,
	rsrx_timer_id_t eTimerId)
{
	rsrx_timer_command_t xCommand;

	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->xPlatformAdapter.xPlatformPorts.xTimer.pfCommand ==
			(rsrx_timer_command_fn)0) ||
		(eTimerId == RSRX_TIMER_ID_INVALID))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	xCommand.eTimerId = eTimerId;
	xCommand.eCommandType = RSRX_TIMER_COMMAND_CANCEL;
	xCommand.uDeadlineNs = 0U;
	xCommand.eReason = RSRX_REASON_NONE;

	if(pxSession->xPlatformAdapter.xPlatformPorts.xTimer.pfCommand(
		pxSession->xPlatformAdapter.xPlatformPorts.xTimer.pvContext,
		&xCommand) != RSRX_PLATFORM_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

static rsrx_status_t eCancelSessionRuntimeTimers(
	rsrx_session_t * pxSession)
{
	if(eCancelSessionTimer(pxSession, RSRX_TIMER_ID_SUPERVISION) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eCancelSessionTimer(pxSession, RSRX_TIMER_ID_RETRANSMISSION) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

static rsrx_status_t eEnterSessionCriticalSection(
	const rsrx_session_t * pxSession)
{
	const rsrx_critical_section_port_t * pxCriticalSection;

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxCriticalSection = &pxSession->xPlatformAdapter.xPlatformPorts.xCriticalSection;
	if(pxCriticalSection->pfEnter == (rsrx_critical_section_enter_fn)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(pxCriticalSection->pfEnter(pxCriticalSection->pvContext) != RSRX_PLATFORM_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

static rsrx_status_t eExitSessionCriticalSection(
	const rsrx_session_t * pxSession)
{
	const rsrx_critical_section_port_t * pxCriticalSection;

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxCriticalSection = &pxSession->xPlatformAdapter.xPlatformPorts.xCriticalSection;
	if(pxCriticalSection->pfExit == (rsrx_critical_section_exit_fn)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(pxCriticalSection->pfExit(pxCriticalSection->pvContext) != RSRX_PLATFORM_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

static void vNotifyDirectReject(
	rsrx_session_t * pxSession,
	rsrx_reason_code_t eReason,
	rsrx_diagnostic_code_t eDiagnostic)
{
	if(pxSession == (rsrx_session_t *)0)
	{
		return;
	}

	vSetDirectReport(pxSession, RSRX_STATUS_REJECTED, eReason, eDiagnostic);
	if(eDiagnostic == RSRX_DIAG_ERROR_INTERFACE)
	{
		rsrx_transport_adapter_note_busy_reject_escalation(
			&pxSession->xTransportAdapter);
	}
	vWriteDirectDiagnostic(pxSession);
	if(pxSession->pfApiNotification != (rsrx_api_notification_fn)0)
	{
		pxSession->pfApiNotification(
			pxSession->pvApiCallbackContext,
			&pxSession->xLastReport);
	}
}

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

static void vApplicationExecutorDispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	rsrx_session_t * pxSession = (rsrx_session_t *)pvContext;
	const rsrx_decoded_message_t * pxMessage;
	rsrx_application_data_indication_t xIndication;
	(void)pxTransition;
	(void)eAction;
	(void)uActionIndex;

	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->pfApplicationData == (rsrx_application_data_fn)0))
	{
		return;
	}

	pxMessage = rsrx_transport_adapter_get_last_inbound_message(
		&pxSession->xTransportAdapter);
	if(pxMessage == (const rsrx_decoded_message_t *)0)
	{
		return;
	}

	xIndication.puPayload = pxMessage->auPayload;
	xIndication.xPayloadLength = pxMessage->xPayloadLength;
	xIndication.eReason = pxMessage->eReason;
	xIndication.uSequenceNumber = pxMessage->uSequenceNumber;
	xIndication.uConfirmationNumber = pxMessage->uConfirmationNumber;

	pxSession->pfApplicationData(
		pxSession->pvApplicationDataContext,
		&pxSession->xLastReport,
		&xIndication);
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
		(eAction == RSRX_ACTION_CLEAR_RETRANSMISSION_CONTEXT))
	{
		rsrx_transport_adapter_clear_retransmission_context(
			&pxSession->xTransportAdapter);
	}

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

	if(ppxReport != (const rsrx_orchestrator_report_t **)0)
	{
		*ppxReport = (const rsrx_orchestrator_report_t *)0;
	}

	if((pxSession == (rsrx_session_t *)0) ||
		(ppxReport == (const rsrx_orchestrator_report_t **)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	eStatus = rsrx_orchestrator_process_event(
		&pxSession->xOrchestrator,
		eEvent,
		&pxSession->xLastReport);

	if((eStatus == RSRX_STATUS_OK) || (eStatus == RSRX_STATUS_REJECTED))
	{
		switch(eEvent)
		{
			case RSRX_EVENT_VALID_INBOUND_CONNECT:
			case RSRX_EVENT_HANDSHAKE_SUCCESS:
			case RSRX_EVENT_VALID_HEARTBEAT:
			case RSRX_EVENT_VALID_DATA:
			case RSRX_EVENT_RECOVERY_SUCCESS:
				rsrx_transport_adapter_clear_outstanding_send(
					&pxSession->xTransportAdapter);
				break;

			case RSRX_EVENT_INIT_SUCCESS:
			case RSRX_EVENT_INIT_FAILURE:
			case RSRX_EVENT_CONNECT_REQUEST:
			case RSRX_EVENT_SEQUENCE_GAP_DETECTED:
			case RSRX_EVENT_DISCONNECT_REQUEST:
			case RSRX_EVENT_TIMEOUT:
			case RSRX_EVENT_INVALID_MESSAGE:
			case RSRX_EVENT_VERSION_MISMATCH:
			case RSRX_EVENT_PROTOCOL_ERROR:
			case RSRX_EVENT_RETRANSMISSION_FAILURE:
			case RSRX_EVENT_INVALID_RESPONSE:
			case RSRX_EVENT_SHUTDOWN_REQUEST:
			case RSRX_EVENT_CLEANUP_COMPLETE:
			case RSRX_EVENT_INVALID:
			default:
				break;
		}
	}

	*ppxReport = &pxSession->xLastReport;

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		*ppxReport = (const rsrx_orchestrator_report_t *)0;
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

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
	rsrx_action_executor_t xApplicationExecutor;
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
		&pxSession->xChannelManager,
		pxConfig->eDefaultChannelId,
		pxConfig->puFramePayload,
		pxConfig->xFramePayloadLength) != RSRX_TRANSPORT_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(rsrx_channel_manager_init(
		&pxSession->xChannelManager,
		&pxConfig->xChannelManagerConfig) != RSRX_CHANNEL_MANAGER_STATUS_OK)
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
	pxSession->pvApplicationDataContext = pxConfig->pvApplicationDataContext;
	pxSession->pfApplicationData = pxConfig->pfApplicationData;
	pxSession->pvLifecycleCallbackContext = pxConfig->pvLifecycleCallbackContext;
	pxSession->pfLifecycleNotification = pxConfig->pfLifecycleNotification;
	pxSession->uBusyRejectErrorThreshold = pxConfig->uBusyRejectErrorThreshold;

	xApplicationExecutor.pvContext = pxSession;
	xApplicationExecutor.pfDispatch = vApplicationExecutorDispatch;
	xApiExecutor.pvContext = pxSession;
	xApiExecutor.pfDispatch = vApiExecutorDispatch;
	xLifecycleExecutor.pvContext = pxSession;
	xLifecycleExecutor.pfDispatch = vLifecycleExecutorDispatch;

	if(rsrx_platform_adapter_build_executor_table(
		&pxSession->xExecutors,
		&pxSession->xTransportAdapter,
		&pxSession->xPlatformAdapter,
		&xApplicationExecutor,
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
	vResetLastReport(pxSession);

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

	if(ppxReport != (const rsrx_orchestrator_report_t **)0)
	{
		*ppxReport = (const rsrx_orchestrator_report_t *)0;
	}

	if(eMapTimerSourceToEvent(eTimerSource, &eEvent) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return eProcessSessionEvent(pxSession, eEvent, ppxReport);
}

rsrx_status_t rsrx_session_resolve_inbound_event(
	const rsrx_session_t * pxSession,
	const rsrx_decoded_message_t * pxMessage,
	rsrx_event_t * peEvent)
{
	rsrx_status_t eStatus;

	if(peEvent != (rsrx_event_t *)0)
	{
		*peEvent = RSRX_EVENT_INVALID;
	}

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U) ||
		(pxMessage == (const rsrx_decoded_message_t *)0) ||
		(peEvent == (rsrx_event_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	eStatus = rsrx_protocol_context_resolve_inbound_event(
		&pxSession->xTransportAdapter.xProtocolContext,
		pxMessage,
		peEvent);

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		*peEvent = RSRX_EVENT_INVALID;
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return eStatus;
}

rsrx_status_t rsrx_session_record_inbound_message(
	rsrx_session_t * pxSession,
	const rsrx_decoded_message_t * pxMessage)
{
	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U) ||
		(pxMessage == (const rsrx_decoded_message_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	rsrx_transport_adapter_record_inbound_message(
		&pxSession->xTransportAdapter,
		pxMessage);

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_session_clear_outstanding_send_on_feedback(
	rsrx_session_t * pxSession)
{
	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	rsrx_transport_adapter_clear_outstanding_send_on_feedback(
		&pxSession->xTransportAdapter);

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_session_send_application_data(
	rsrx_session_t * pxSession,
	const uint8_t * puPayload,
	size_t xPayloadLength)
{
	rsrx_transport_status_t eSendStatus;
	rsrx_status_t eStatus;

	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U) ||
		((puPayload == (const uint8_t *)0) && (xPayloadLength > 0U)))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(rsrx_orchestrator_get_state(&pxSession->xOrchestrator) != RSRX_STATE_ESTABLISHED)
	{
		if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
		{
			return RSRX_STATUS_INVALID_ARGUMENT;
		}
		return RSRX_STATUS_INVALID_STATE;
	}

	eSendStatus = rsrx_transport_adapter_send_application_data(
		&pxSession->xTransportAdapter,
		puPayload,
		xPayloadLength);
	if(eSendStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		vNotifyDirectReject(
			pxSession,
			RSRX_REASON_APPLICATION_DATA_REQUESTED,
			eResolveBusyRejectDiagnostic(pxSession));
		eStatus = RSRX_STATUS_REJECTED;
	}
	else
	{
		eStatus = RSRX_STATUS_OK;
	}

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return eStatus;
}

const rsrx_outbound_send_telemetry_t * rsrx_session_get_outbound_telemetry(
	const rsrx_session_t * pxSession)
{
	const rsrx_outbound_send_telemetry_t * pxTelemetry;

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return (const rsrx_outbound_send_telemetry_t *)0;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return (const rsrx_outbound_send_telemetry_t *)0;
	}

	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(
		&pxSession->xTransportAdapter);

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return (const rsrx_outbound_send_telemetry_t *)0;
	}

	return pxTelemetry;
}

rsrx_status_t rsrx_session_copy_outbound_telemetry(
	const rsrx_session_t * pxSession,
	rsrx_outbound_send_telemetry_t * pxTelemetry)
{
	const rsrx_outbound_send_telemetry_t * pxSource;

	if(pxTelemetry != (rsrx_outbound_send_telemetry_t *)0)
	{
		vClearOutboundTelemetry(pxTelemetry);
	}

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U) ||
		(pxTelemetry == (rsrx_outbound_send_telemetry_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxSource = rsrx_transport_adapter_get_outbound_telemetry(
		&pxSession->xTransportAdapter);
	if(pxSource == (const rsrx_outbound_send_telemetry_t *)0)
	{
		(void)eExitSessionCriticalSection(pxSession);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	*pxTelemetry = *pxSource;

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		vClearOutboundTelemetry(pxTelemetry);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_session_copy_outbound_queue_snapshot(
	const rsrx_session_t * pxSession,
	rsrx_outbound_queue_snapshot_t * pxSnapshot)
{
	const rsrx_outbound_send_telemetry_t * pxTelemetry;

	vClearOutboundQueueSnapshot(pxSnapshot);

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U) ||
		(pxSnapshot == (rsrx_outbound_queue_snapshot_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(
		&pxSession->xTransportAdapter);
	if(pxTelemetry == (const rsrx_outbound_send_telemetry_t *)0)
	{
		(void)eExitSessionCriticalSection(pxSession);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxSnapshot->uOutstandingSendPresent =
		rsrx_transport_adapter_has_outstanding_send(
			&pxSession->xTransportAdapter);
	if(pxSnapshot->uOutstandingSendPresent != 0U)
	{
		pxSnapshot->eOutstandingSendChannelId =
			rsrx_transport_adapter_get_outstanding_send_channel(
				&pxSession->xTransportAdapter);
	}
	pxSnapshot->uDeferredSendPresent =
		pxSession->xTransportAdapter.uHasDeferredSend;
	pxSnapshot->uDeferredSendCount =
		pxSession->xTransportAdapter.uDeferredSendCount;
	pxSnapshot->xTelemetry = *pxTelemetry;

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		vClearOutboundQueueSnapshot(pxSnapshot);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_session_copy_channel_manager_snapshot(
	const rsrx_session_t * pxSession,
	rsrx_channel_manager_snapshot_t * pxSnapshot)
{
	const rsrx_channel_manager_context_t * pxChannelManager;

	vClearChannelManagerSnapshot(pxSnapshot);

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U) ||
		(pxSnapshot == (rsrx_channel_manager_snapshot_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxChannelManager = &pxSession->xChannelManager;
	if(pxChannelManager->uInitialized == 0U)
	{
		(void)eExitSessionCriticalSection(pxSession);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(pxChannelManager->uActiveChannelIndex < pxChannelManager->xConfig.uChannelCount)
	{
		pxSnapshot->eActiveChannelId =
			pxChannelManager->xConfig.axChannels[
				pxChannelManager->uActiveChannelIndex].eChannelId;
	}
	if(pxChannelManager->xConfig.uPreferredChannelIndex <
		pxChannelManager->xConfig.uChannelCount)
	{
		pxSnapshot->ePreferredChannelId =
			pxChannelManager->xConfig.axChannels[
				pxChannelManager->xConfig.uPreferredChannelIndex].eChannelId;
	}
	pxSnapshot->uAvailableChannelCount =
		uCountChannelManagerAvailableChannels(pxChannelManager);
	pxSnapshot->uLastSelectionWasFailover =
		pxChannelManager->uLastSelectionWasFailover;
	pxSnapshot->uPreferredRecoveryStableSelectionCount =
		pxChannelManager->uPreferredRecoveryStableSelectionCount;
	pxSnapshot->uPreferredRecoveryPendingPenaltySelections =
		pxChannelManager->uPreferredRecoveryPendingPenaltySelections;
	pxSnapshot->uPreferredRecoveryPenaltyArmCount =
		pxChannelManager->uPreferredRecoveryPenaltyArmCount;
	pxSnapshot->uPreferredRecoveryPenaltyRearmCount =
		pxChannelManager->uPreferredRecoveryPenaltyRearmCount;
	pxSnapshot->uPreferredRecoveryPenaltyAppliedCycleCount =
		pxChannelManager->uPreferredRecoveryPenaltyAppliedCycleCount;
	pxSnapshot->uPreferredRecoveryPenaltyAbortCount =
		pxChannelManager->uPreferredRecoveryPenaltyAbortCount;
	pxSnapshot->uPreferredRecoveryPenaltyClearCount =
		pxChannelManager->uPreferredRecoveryPenaltyClearCount;
	pxSnapshot->uPreferredRecoveryPenaltyBypassClearCount =
		pxChannelManager->uPreferredRecoveryPenaltyBypassClearCount;
	pxSnapshot->uPreferredRecoveryPenaltyResetClearCount =
		pxChannelManager->uPreferredRecoveryPenaltyResetClearCount;
	pxSnapshot->uPreferredRecoveryHoldoffTargetCount =
		uGetChannelManagerEffectiveHoldoffTarget(pxChannelManager);
	pxSnapshot->uPreferredRecoveryHoldoffRemainingCount =
		(pxSnapshot->uPreferredRecoveryHoldoffTargetCount >
			pxSnapshot->uPreferredRecoveryStableSelectionCount) ?
			(pxSnapshot->uPreferredRecoveryHoldoffTargetCount -
				pxSnapshot->uPreferredRecoveryStableSelectionCount) :
			0U;
	pxSnapshot->uTotalSwitchCount =
		pxChannelManager->uTotalSwitchCount;
	pxSnapshot->uUnavailableSelectionCount =
		pxChannelManager->uUnavailableSelectionCount;

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		vClearChannelManagerSnapshot(pxSnapshot);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_STATUS_OK;
}

rsrx_state_t rsrx_session_get_state(
	const rsrx_session_t * pxSession)
{
	rsrx_state_t eState;

	if((pxSession == (const rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATE_INVALID;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATE_INVALID;
	}

	eState = rsrx_orchestrator_get_state(&pxSession->xOrchestrator);

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATE_INVALID;
	}

	return eState;
}

rsrx_status_t rsrx_session_reset(
	rsrx_session_t * pxSession)
{
	rsrx_channel_manager_status_t eChannelStatus;
	rsrx_status_t eResetStatus;

	if((pxSession == (rsrx_session_t *)0) ||
		(pxSession->uInitialized == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eEnterSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(eCancelSessionRuntimeTimers(pxSession) != RSRX_STATUS_OK)
	{
		(void)eExitSessionCriticalSection(pxSession);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	rsrx_transport_adapter_reset_runtime_state(&pxSession->xTransportAdapter);
	eChannelStatus = rsrx_channel_manager_reset(&pxSession->xChannelManager);
	if(eChannelStatus != RSRX_CHANNEL_MANAGER_STATUS_OK)
	{
		(void)eExitSessionCriticalSection(pxSession);
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	eResetStatus = rsrx_orchestrator_reset(&pxSession->xOrchestrator);
	if(eResetStatus == RSRX_STATUS_OK)
	{
		vResetLastReport(pxSession);
	}

	if(eExitSessionCriticalSection(pxSession) != RSRX_STATUS_OK)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	return eResetStatus;
}
