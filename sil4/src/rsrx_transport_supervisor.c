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
	pxReport->pxLastReport = (const rsrx_orchestrator_report_t *)0;
	pxReport->uProcessedFrameCount = 0U;
	pxReport->uPollCount = 0U;
	pxReport->uConsecutiveSendFailureCount = 0U;
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
	pxContext->xLastReport.eLastDecision =
		(eSessionStatus == RSRX_STATUS_REJECTED) ?
			RSRX_SUPERVISOR_DECISION_SESSION_REJECTED :
			RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED;

	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}

static void vResetSendFailureBudget(
	rsrx_transport_supervisor_context_t * pxContext)
{
	pxContext->xLastReport.uConsecutiveSendFailureCount = 0U;
}

static uint32_t uSendFailureBudgetExceeded(
	rsrx_transport_supervisor_context_t * pxContext)
{
	if(pxContext->xLastReport.uConsecutiveSendFailureCount < UINT32_MAX)
	{
		pxContext->xLastReport.uConsecutiveSendFailureCount++;
	}

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
		pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_DECODE_FAILED;
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

	vResetSendFailureBudget(pxContext);

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
	pxContext->xLastReport.eLastDecision =
		(eSessionStatus == RSRX_STATUS_REJECTED) ?
			RSRX_SUPERVISOR_DECISION_SESSION_REJECTED :
			RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED;

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
	pxContext->xLastReport.uPollCount++;
	if(eTransportStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN;
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
		pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN;
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
		pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE;
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_NO_FRAME;
	}

	if(eTransportStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN;
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
		pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE;
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_NO_FRAME;
	}

	return eProcessFrameInternal(pxContext, &xFrame, ppxReport);
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
			vResetSendFailureBudget(pxContext);
			pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED;
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;

		case RSRX_TRANSPORT_EVENT_SEND_FAILED:
			if(uSendFailureBudgetExceeded(pxContext) == 0U)
			{
				pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED;
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			vResetSendFailureBudget(pxContext);
			pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_SEND_FAILURE_ESCALATED;
			return eProcessSessionEventInternal(
				pxContext,
				RSRX_EVENT_PROTOCOL_ERROR,
				ppxReport);

		case RSRX_TRANSPORT_EVENT_CHANNEL_DOWN:
			vResetSendFailureBudget(pxContext);
			pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_ESCALATED;
			return eProcessSessionEventInternal(
				pxContext,
				RSRX_EVENT_PROTOCOL_ERROR,
				ppxReport);

		case RSRX_TRANSPORT_EVENT_FRAME_RECEIVED:
			return eProcessFrameInternal(pxContext, pxFrame, ppxReport);

		case RSRX_TRANSPORT_EVENT_CHANNEL_UP:
		case RSRX_TRANSPORT_EVENT_NONE:
		default:
			pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_TRANSPORT_EVENT_IGNORED;
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
	pxContext->xLastReport.eLastDecision = RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED;
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}

	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}
