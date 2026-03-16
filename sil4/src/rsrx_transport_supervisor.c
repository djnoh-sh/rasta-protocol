#include "rsrx_transport_supervisor.h"

static void vResetSupervisorReport(
	rsrx_transport_supervisor_report_t * pxReport)
{
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
	pxReport->pxLastReport = (const rsrx_orchestrator_report_t *)0;
	pxReport->uProcessedFrameCount = 0U;
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
	pxContext->uInitialized = 1U;

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_frame(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_codec_status_t eCodecStatus;
	rsrx_status_t eSessionStatus;
	rsrx_event_t eInboundEvent;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxFrame == (const rsrx_transport_frame_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xLastReport.xLastFrame = *pxFrame;
	eCodecStatus = pxContext->xCodec.pfDecode(pxFrame, &pxContext->xLastReport.xLastMessage);
	if(eCodecStatus != RSRX_CODEC_STATUS_OK)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_DECODE_FAILED;
	}

	eInboundEvent = eResolveInboundEvent(
		pxContext,
		&pxContext->xLastReport.xLastMessage);
	if(eInboundEvent == pxContext->xLastReport.xLastMessage.eSuggestedEvent)
	{
		rsrx_transport_adapter_record_inbound_message(
			&pxContext->pxSession->xTransportAdapter,
			&pxContext->xLastReport.xLastMessage);
	}

	eSessionStatus = rsrx_session_process_event(
		pxContext->pxSession,
		eInboundEvent,
		&pxContext->xLastReport.pxLastReport);
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}

	pxContext->xLastReport.uProcessedFrameCount++;
	*ppxReport = &pxContext->xLastReport;

	return RSRX_SUPERVISOR_STATUS_OK;
}
