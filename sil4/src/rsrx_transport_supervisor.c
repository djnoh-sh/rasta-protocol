#include "rsrx_transport_supervisor.h"

#define D_RSRX_SUPERVISOR_DEFAULT_SEND_FAILURE_BUDGET (2U)

static void vResetSupervisorReport(
	rsrx_transport_supervisor_report_t * pxReport)
{
	pxReport->xLastChannelState.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->xLastChannelState.uIsAvailable = 0U;
	pxReport->xLastFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->xLastFrame.puPayload = (const uint8_t *)0;
	pxReport->xLastFrame.xPayloadLength = 0U;
	pxReport->xLastFrame.eEventType = RSRX_TRANSPORT_EVENT_NONE;
	pxReport->xLastMessage.eMessageType = RSRX_MESSAGE_TYPE_INVALID;
	pxReport->xLastMessage.eSuggestedEvent = RSRX_EVENT_INVALID;
	pxReport->xLastMessage.eReason = RSRX_REASON_NONE;
	pxReport->xLastMessage.uSequenceNumber = 0U;
	pxReport->xLastMessage.uConfirmationNumber = 0U;
	pxReport->xLastMessage.xPayloadLength = 0U;
	pxReport->eLastEffectiveEvent = RSRX_EVENT_INVALID;
	pxReport->eLastSessionStatus = RSRX_STATUS_OK;
	pxReport->eLastDecision = RSRX_SUPERVISOR_DECISION_NONE;
	pxReport->eLastDecisionClass = RSRX_SUPERVISOR_DECISION_CLASS_NONE;
	pxReport->eLastBudgetUpdate = RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
	pxReport->eBudgetChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->pxLastReport = (const rsrx_orchestrator_report_t *)0;
	pxReport->uProcessedFrameCount = 0U;
	pxReport->uPollCount = 0U;
	pxReport->uConsecutiveSendFailureCount = 0U;
	pxReport->uSendFailureBudgetResetCount = 0U;
	pxReport->uAcceptedDecisionCount = 0U;
	pxReport->uRejectedDecisionCount = 0U;
	pxReport->uIgnoredDecisionCount = 0U;
	pxReport->uErrorDecisionCount = 0U;
	pxReport->uChannelSwitchCount = 0U;
	pxReport->uLastChannelSwitchOccurred = 0U;
	pxReport->uLastPumpIterationCount = 0U;
	pxReport->uLastPumpProcessedFrameCount = 0U;
}

static rsrx_supervisor_decision_class_t eMapDecisionClass(
	rsrx_supervisor_decision_t eDecision)
{
	switch(eDecision)
	{
		case RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED:
		case RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED:
			return RSRX_SUPERVISOR_DECISION_CLASS_ACCEPTED;

		case RSRX_SUPERVISOR_DECISION_SESSION_REJECTED:
			return RSRX_SUPERVISOR_DECISION_CLASS_REJECTED;

		case RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE:
		case RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED:
		case RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED:
		case RSRX_SUPERVISOR_DECISION_SEND_FAILURE_INACTIVE_CHANNEL_IGNORED:
		case RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED:
		case RSRX_SUPERVISOR_DECISION_TRANSPORT_EVENT_IGNORED:
			return RSRX_SUPERVISOR_DECISION_CLASS_IGNORED;

		case RSRX_SUPERVISOR_DECISION_DECODE_FAILED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN:
		case RSRX_SUPERVISOR_DECISION_SEND_FAILURE_ESCALATED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_ESCALATED:
			return RSRX_SUPERVISOR_DECISION_CLASS_ERROR;

		case RSRX_SUPERVISOR_DECISION_NONE:
		default:
			return RSRX_SUPERVISOR_DECISION_CLASS_NONE;
	}
}

static void vRecordDecision(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_supervisor_decision_t eDecision)
{
	rsrx_supervisor_decision_class_t eDecisionClass;

	pxContext->xLastReport.eLastDecision = eDecision;
	eDecisionClass = eMapDecisionClass(eDecision);
	pxContext->xLastReport.eLastDecisionClass = eDecisionClass;

	switch(eDecisionClass)
	{
		case RSRX_SUPERVISOR_DECISION_CLASS_ACCEPTED:
			if(pxContext->xLastReport.uAcceptedDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uAcceptedDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_REJECTED:
			if(pxContext->xLastReport.uRejectedDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uRejectedDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_IGNORED:
			if(pxContext->xLastReport.uIgnoredDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uIgnoredDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_ERROR:
			if(pxContext->xLastReport.uErrorDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uErrorDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_NONE:
		default:
			break;
	}
}

static void vRefreshChannelSwitchTelemetry(
	rsrx_transport_supervisor_context_t * pxContext)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (rsrx_session_t *)0))
	{
		return;
	}

	pxContext->xLastReport.uChannelSwitchCount =
		pxContext->pxSession->xChannelManager.uTotalSwitchCount;
	pxContext->xLastReport.uLastChannelSwitchOccurred =
		pxContext->pxSession->xChannelManager.uLastSelectionWasFailover;
}

static rsrx_transport_channel_id_t eGetActiveChannelId(
	const rsrx_transport_supervisor_context_t * pxContext)
{
	if((pxContext == (const rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (const rsrx_session_t *)0))
	{
		return RSRX_TRANSPORT_CHANNEL_INVALID;
	}

	return rsrx_channel_manager_get_active_channel(
		&pxContext->pxSession->xChannelManager);
}

static rsrx_event_t eResolveInboundEvent(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage)
{
	rsrx_event_t eResolvedEvent;

	if(rsrx_protocol_context_resolve_inbound_event(
		&pxContext->pxSession->xTransportAdapter.xProtocolContext,
		pxMessage,
		&eResolvedEvent) != RSRX_STATUS_OK)
	{
		return RSRX_EVENT_PROTOCOL_ERROR;
	}

	return eResolvedEvent;
}

static uint32_t uSessionStatusIsHandled(
	rsrx_status_t eStatus)
{
	return (uint32_t)((eStatus == RSRX_STATUS_OK) ||
		(eStatus == RSRX_STATUS_REJECTED));
}

static rsrx_supervisor_status_t eProcessSessionEventInternal(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_event_t eEvent,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_status_t eSessionStatus;

	eSessionStatus = rsrx_session_process_event(
		pxContext->pxSession,
		eEvent,
		&pxContext->xLastReport.pxLastReport);
	pxContext->xLastReport.eLastEffectiveEvent = eEvent;
	pxContext->xLastReport.eLastSessionStatus = eSessionStatus;
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}
	vRecordDecision(
		pxContext,
		(eSessionStatus == RSRX_STATUS_REJECTED) ?
			RSRX_SUPERVISOR_DECISION_SESSION_REJECTED :
			RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED);
	vRefreshChannelSwitchTelemetry(pxContext);

	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}

static void vResetSendFailureBudget(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_supervisor_budget_update_t eBudgetUpdate)
{
	if(pxContext->xLastReport.uConsecutiveSendFailureCount > 0U)
	{
		pxContext->xLastReport.uConsecutiveSendFailureCount = 0U;
		if(pxContext->xLastReport.uSendFailureBudgetResetCount < UINT32_MAX)
		{
			pxContext->xLastReport.uSendFailureBudgetResetCount++;
		}
		pxContext->xLastReport.eLastBudgetUpdate = eBudgetUpdate;
		pxContext->xLastReport.eBudgetChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	}
	else
	{
		pxContext->xLastReport.eLastBudgetUpdate = RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
	}
}

static uint32_t uAlternativeChannelIsAvailable(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eFailedChannelId)
{
	rsrx_transport_status_t eTransportStatus;
	rsrx_transport_channel_state_t xChannelState;

	eTransportStatus = rsrx_transport_adapter_query_channel(
		&pxContext->pxSession->xTransportAdapter,
		&xChannelState);
	if((eTransportStatus != RSRX_TRANSPORT_STATUS_OK) ||
		(xChannelState.uIsAvailable == 0U))
	{
		return 0U;
	}

	pxContext->xLastReport.xLastChannelState = xChannelState;
	vRefreshChannelSwitchTelemetry(pxContext);
	return (uint32_t)(xChannelState.eChannelId != eFailedChannelId);
}

static uint32_t uRefreshAvailableChannelState(
	rsrx_transport_supervisor_context_t * pxContext)
{
	rsrx_transport_status_t eTransportStatus;
	rsrx_transport_channel_state_t xChannelState;

	eTransportStatus = rsrx_transport_adapter_query_channel(
		&pxContext->pxSession->xTransportAdapter,
		&xChannelState);
	if((eTransportStatus != RSRX_TRANSPORT_STATUS_OK) ||
		(xChannelState.uIsAvailable == 0U))
	{
		return 0U;
	}

	pxContext->xLastReport.xLastChannelState = xChannelState;
	vRefreshChannelSwitchTelemetry(pxContext);
	return 1U;
}

static uint32_t uFrameTargetsActiveChannel(
	const rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId)
{
	rsrx_transport_channel_id_t eActiveChannelId;

	eActiveChannelId = eGetActiveChannelId(pxContext);

	return (uint32_t)((eActiveChannelId == RSRX_TRANSPORT_CHANNEL_INVALID) ||
		(eActiveChannelId == eChannelId));
}

static uint32_t uFrameMatchesOutstandingSend(
	const rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId)
{
	rsrx_transport_channel_id_t eOutstandingChannelId;

	if((pxContext == (const rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (const rsrx_session_t *)0))
	{
		return 0U;
	}

	if(rsrx_transport_adapter_has_outstanding_send(
		&pxContext->pxSession->xTransportAdapter) == 0U)
	{
		return 0U;
	}

	eOutstandingChannelId = rsrx_transport_adapter_get_outstanding_send_channel(
		&pxContext->pxSession->xTransportAdapter);

	return (uint32_t)(eOutstandingChannelId == eChannelId);
}

static uint32_t uSendFailureBudgetExceeded(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eActiveChannelId)
{
	if((pxContext->xLastReport.eBudgetChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(pxContext->xLastReport.eBudgetChannelId != eActiveChannelId) &&
		(pxContext->xLastReport.uConsecutiveSendFailureCount > 0U))
	{
		pxContext->xLastReport.uConsecutiveSendFailureCount = 0U;
		if(pxContext->xLastReport.uSendFailureBudgetResetCount < UINT32_MAX)
		{
			pxContext->xLastReport.uSendFailureBudgetResetCount++;
		}
		pxContext->xLastReport.eLastBudgetUpdate =
			RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_AND_INCREMENT_ON_CHANNEL_SWITCH;
	}
	else
	{
		pxContext->xLastReport.eLastBudgetUpdate =
			RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED;
	}

	if(pxContext->xLastReport.uConsecutiveSendFailureCount < UINT32_MAX)
	{
		pxContext->xLastReport.uConsecutiveSendFailureCount++;
	}
	pxContext->xLastReport.eBudgetChannelId = eActiveChannelId;

	return (uint32_t)(pxContext->xLastReport.uConsecutiveSendFailureCount >=
		pxContext->uMaxConsecutiveSendFailures);
}

static rsrx_supervisor_status_t eProcessFrameInternal(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_codec_status_t eCodecStatus;
	rsrx_status_t eSessionStatus;
	rsrx_event_t eInboundEvent;

	pxContext->xLastReport.xLastFrame = *pxFrame;
	eCodecStatus = pxContext->xCodec.pfDecode(pxFrame, &pxContext->xLastReport.xLastMessage);
	if(eCodecStatus != RSRX_CODEC_STATUS_OK)
	{
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_DECODE_FAILED);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_DECODE_FAILED;
	}

	eInboundEvent = eResolveInboundEvent(
		pxContext,
		&pxContext->xLastReport.xLastMessage);
	pxContext->xLastReport.eLastEffectiveEvent = eInboundEvent;
	if((eInboundEvent == pxContext->xLastReport.xLastMessage.eSuggestedEvent) ||
		(eInboundEvent == RSRX_EVENT_RECOVERY_SUCCESS))
	{
		rsrx_transport_adapter_record_inbound_message(
			&pxContext->pxSession->xTransportAdapter,
			&pxContext->xLastReport.xLastMessage);
	}

	vResetSendFailureBudget(
		pxContext,
		RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_INBOUND_FRAME);

	eSessionStatus = rsrx_session_process_event(
		pxContext->pxSession,
		eInboundEvent,
		&pxContext->xLastReport.pxLastReport);
	pxContext->xLastReport.eLastSessionStatus = eSessionStatus;
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}
	vRecordDecision(
		pxContext,
		(eSessionStatus == RSRX_STATUS_REJECTED) ?
			RSRX_SUPERVISOR_DECISION_SESSION_REJECTED :
			RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED);
	vRefreshChannelSwitchTelemetry(pxContext);

	pxContext->xLastReport.uProcessedFrameCount++;
	*ppxReport = &pxContext->xLastReport;

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_init(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_session_t * pxSession,
	const rsrx_codec_port_t * pxCodec)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxSession == (rsrx_session_t *)0) ||
		(pxCodec == (const rsrx_codec_port_t *)0) ||
		(pxCodec->pfDecode == (rsrx_decode_frame_fn)0))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->pxSession = pxSession;
	pxContext->xCodec = *pxCodec;
	vResetSupervisorReport(&pxContext->xLastReport);
	pxContext->uMaxConsecutiveSendFailures = D_RSRX_SUPERVISOR_DEFAULT_SEND_FAILURE_BUDGET;
	pxContext->uInitialized = 1U;

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_frame(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxFrame == (const rsrx_transport_frame_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	return eProcessFrameInternal(pxContext, pxFrame, ppxReport);
}

rsrx_supervisor_status_t rsrx_transport_supervisor_poll_receive(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_transport_status_t eTransportStatus;
	rsrx_transport_frame_t xFrame;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	eTransportStatus = rsrx_transport_adapter_query_channel(
		&pxContext->pxSession->xTransportAdapter,
		&pxContext->xLastReport.xLastChannelState);
	vRefreshChannelSwitchTelemetry(pxContext);
	pxContext->xLastReport.uPollCount++;
	if(eTransportStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN);
		*ppxReport = &pxContext->xLastReport;
		if((eTransportStatus == RSRX_TRANSPORT_STATUS_CHANNEL_DOWN) ||
			(eTransportStatus == RSRX_TRANSPORT_STATUS_UNAVAILABLE))
		{
			return RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN;
		}

		return RSRX_SUPERVISOR_STATUS_RECEIVE_ERROR;
	}

	if(pxContext->xLastReport.xLastChannelState.uIsAvailable == 0U)
	{
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN;
	}

	xFrame.eChannelId = pxContext->xLastReport.xLastChannelState.eChannelId;
	xFrame.puPayload = (const uint8_t *)0;
	xFrame.xPayloadLength = 0U;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_NONE;

	eTransportStatus = rsrx_transport_adapter_receive_frame(
		&pxContext->pxSession->xTransportAdapter,
		&xFrame);
	if(eTransportStatus == RSRX_TRANSPORT_STATUS_UNAVAILABLE)
	{
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_NO_FRAME;
	}

	if(eTransportStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN);
		*ppxReport = &pxContext->xLastReport;
		if(eTransportStatus == RSRX_TRANSPORT_STATUS_CHANNEL_DOWN)
		{
			return RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN;
		}

		return RSRX_SUPERVISOR_STATUS_RECEIVE_ERROR;
	}

	if(xFrame.eEventType != RSRX_TRANSPORT_EVENT_FRAME_RECEIVED)
	{
		pxContext->xLastReport.xLastFrame = xFrame;
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_NO_FRAME;
	}

	return eProcessFrameInternal(pxContext, &xFrame, ppxReport);
}

rsrx_supervisor_status_t rsrx_transport_supervisor_pump_receive(
	rsrx_transport_supervisor_context_t * pxContext,
	uint32_t uMaxPolls,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_supervisor_status_t eStatus;
	uint32_t uInitialProcessedCount;
	uint32_t uIteration;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U) ||
		(uMaxPolls == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xLastReport.uLastPumpIterationCount = 0U;
	pxContext->xLastReport.uLastPumpProcessedFrameCount = 0U;
	uInitialProcessedCount = pxContext->xLastReport.uProcessedFrameCount;

	for(uIteration = 0U; uIteration < uMaxPolls; ++uIteration)
	{
		eStatus = rsrx_transport_supervisor_poll_receive(pxContext, ppxReport);
		pxContext->xLastReport.uLastPumpIterationCount++;

		if(eStatus == RSRX_SUPERVISOR_STATUS_OK)
		{
			continue;
		}

		if(eStatus == RSRX_SUPERVISOR_STATUS_NO_FRAME)
		{
			pxContext->xLastReport.uLastPumpProcessedFrameCount =
				pxContext->xLastReport.uProcessedFrameCount - uInitialProcessedCount;
			*ppxReport = &pxContext->xLastReport;
			return ((pxContext->xLastReport.uLastPumpProcessedFrameCount > 0U) ?
				RSRX_SUPERVISOR_STATUS_OK :
				RSRX_SUPERVISOR_STATUS_NO_FRAME);
		}

		pxContext->xLastReport.uLastPumpProcessedFrameCount =
			pxContext->xLastReport.uProcessedFrameCount - uInitialProcessedCount;
		*ppxReport = &pxContext->xLastReport;
		return eStatus;
	}

	pxContext->xLastReport.uLastPumpProcessedFrameCount =
		pxContext->xLastReport.uProcessedFrameCount - uInitialProcessedCount;
	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_transport_event(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxFrame == (const rsrx_transport_frame_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xLastReport.xLastFrame = *pxFrame;

	switch(pxFrame->eEventType)
	{
		case RSRX_TRANSPORT_EVENT_SEND_COMPLETED:
			if(uFrameMatchesOutstandingSend(pxContext, pxFrame->eChannelId) == 0U)
			{
				pxContext->xLastReport.eLastBudgetUpdate =
					RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			rsrx_transport_adapter_clear_outstanding_send(
				&pxContext->pxSession->xTransportAdapter);
			vResetSendFailureBudget(
				pxContext,
				RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_SEND_COMPLETED);
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;

		case RSRX_TRANSPORT_EVENT_SEND_FAILED:
			if(uFrameMatchesOutstandingSend(pxContext, pxFrame->eChannelId) == 0U)
			{
				pxContext->xLastReport.eLastBudgetUpdate =
					RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			if(uFrameTargetsActiveChannel(pxContext, pxFrame->eChannelId) == 0U)
			{
				pxContext->xLastReport.eLastBudgetUpdate =
					RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_SEND_FAILURE_INACTIVE_CHANNEL_IGNORED);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			if(uSendFailureBudgetExceeded(
				pxContext,
				eGetActiveChannelId(pxContext)) == 0U)
			{
				vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			vResetSendFailureBudget(
				pxContext,
				RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_ESCALATION);
			rsrx_transport_adapter_clear_outstanding_send(
				&pxContext->pxSession->xTransportAdapter);
			return eProcessSessionEventInternal(
				pxContext,
				RSRX_EVENT_PROTOCOL_ERROR,
				ppxReport);

		case RSRX_TRANSPORT_EVENT_CHANNEL_DOWN:
			vResetSendFailureBudget(
				pxContext,
				RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_CHANNEL_DOWN);
			if(uAlternativeChannelIsAvailable(pxContext, pxFrame->eChannelId) != 0U)
			{
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}
			return eProcessSessionEventInternal(
				pxContext,
				RSRX_EVENT_PROTOCOL_ERROR,
				ppxReport);

		case RSRX_TRANSPORT_EVENT_CHANNEL_UP:
			if(uRefreshAvailableChannelState(pxContext) != 0U)
			{
				vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_TRANSPORT_EVENT_IGNORED);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;

		case RSRX_TRANSPORT_EVENT_FRAME_RECEIVED:
			return eProcessFrameInternal(pxContext, pxFrame, ppxReport);

		case RSRX_TRANSPORT_EVENT_NONE:
		default:
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_TRANSPORT_EVENT_IGNORED);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
	}
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_timer_expiry(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_timer_expiry_source_t eTimerSource,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_status_t eSessionStatus;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	eSessionStatus = rsrx_session_process_timer_expiry(
		pxContext->pxSession,
		eTimerSource,
		&pxContext->xLastReport.pxLastReport);
	pxContext->xLastReport.eLastSessionStatus = eSessionStatus;
	vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED);
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}

	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}
