#include "rsrx_platform_adapters.h"

static uint32_t uTransportPortIsValid(
	const rsrx_transport_port_t * pxPort)
{
	return (uint32_t)((pxPort != (const rsrx_transport_port_t *)0) &&
		(pxPort->pfSend != (rsrx_transport_send_fn)0) &&
		(pxPort->pfReceive != (rsrx_transport_receive_fn)0) &&
		(pxPort->pfQueryChannel != (rsrx_transport_channel_query_fn)0));
}

static uint32_t uCodecPortIsValid(
	const rsrx_codec_port_t * pxPort)
{
	return (uint32_t)((pxPort != (const rsrx_codec_port_t *)0) &&
		(pxPort->pfEncode != (rsrx_encode_message_fn)0));
}

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

static uint32_t uActionUsesTransport(
	rsrx_action_t eAction)
{
	return (uint32_t)((eAction == RSRX_ACTION_START_HANDSHAKE) ||
		(eAction == RSRX_ACTION_ACCEPT_INBOUND_CONNECT) ||
		(eAction == RSRX_ACTION_SEND_HEARTBEAT) ||
		(eAction == RSRX_ACTION_REQUEST_RETRANSMISSION) ||
		(eAction == RSRX_ACTION_SEND_DISCONNECT));
}

static rsrx_message_type_t eMapActionToMessageType(
	rsrx_action_t eAction)
{
	switch(eAction)
	{
		case RSRX_ACTION_START_HANDSHAKE:
			return RSRX_MESSAGE_TYPE_CONNECT_REQUEST;

		case RSRX_ACTION_ACCEPT_INBOUND_CONNECT:
			return RSRX_MESSAGE_TYPE_CONNECT_RESPONSE;

		case RSRX_ACTION_SEND_HEARTBEAT:
			return RSRX_MESSAGE_TYPE_HEARTBEAT;

		case RSRX_ACTION_REQUEST_RETRANSMISSION:
			return RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST;

		case RSRX_ACTION_SEND_DISCONNECT:
			return RSRX_MESSAGE_TYPE_DISCONNECT;

		case RSRX_ACTION_NONE:
		default:
			return RSRX_MESSAGE_TYPE_INVALID;
	}
}

static const uint8_t * puResolvePayload(
	const rsrx_transport_adapter_context_t * pxContext,
	rsrx_action_t eAction,
	size_t * pxPayloadLength)
{
	(void)pxContext;
	(void)eAction;

	if(pxPayloadLength == (size_t *)0)
	{
		return (const uint8_t *)0;
	}

	*pxPayloadLength = 0U;
	return (const uint8_t *)0;
}

static rsrx_transport_status_t eRefreshChannelManagerState(
	rsrx_transport_adapter_context_t * pxContext);

static rsrx_transport_status_t eEncodeAndSend(
	rsrx_transport_adapter_context_t * pxContext,
	rsrx_message_type_t eMessageType,
	rsrx_reason_code_t eReason,
	const uint8_t * puPayload,
	size_t xPayloadLength);

static void vRefreshDeferredQueueState(
	rsrx_transport_adapter_context_t * pxContext)
{
	if(pxContext == (rsrx_transport_adapter_context_t *)0)
	{
		return;
	}

	pxContext->uHasDeferredSend = (uint32_t)(pxContext->uDeferredSendCount > 0U);
}

static void vDropDeferredSendFront(
	rsrx_transport_adapter_context_t * pxContext)
{
	if((pxContext == (rsrx_transport_adapter_context_t *)0) ||
		(pxContext->uDeferredSendCount == 0U))
	{
		return;
	}

	if(pxContext->uDeferredSendCount > 1U)
	{
		size_t xIndex;

		pxContext->aeDeferredMessageTypes[0] = pxContext->aeDeferredMessageTypes[1];
		pxContext->aeDeferredReasons[0] = pxContext->aeDeferredReasons[1];
		pxContext->axDeferredPayloadLengths[0] = pxContext->axDeferredPayloadLengths[1];
		for(xIndex = 0U; xIndex < pxContext->axDeferredPayloadLengths[1]; ++xIndex)
		{
			pxContext->aauDeferredPayloads[0][xIndex] =
				pxContext->aauDeferredPayloads[1][xIndex];
		}
	}

	pxContext->uDeferredSendCount--;
	pxContext->aeDeferredMessageTypes[pxContext->uDeferredSendCount] =
		RSRX_MESSAGE_TYPE_INVALID;
	pxContext->aeDeferredReasons[pxContext->uDeferredSendCount] =
		RSRX_REASON_NONE;
	pxContext->axDeferredPayloadLengths[pxContext->uDeferredSendCount] = 0U;
	vRefreshDeferredQueueState(pxContext);
}

static void vDispatchDeferredSendIfPresent(
	rsrx_transport_adapter_context_t * pxContext)
{
	if((pxContext == (rsrx_transport_adapter_context_t *)0) ||
		(pxContext->uHasDeferredSend == 0U))
	{
		return;
	}

	if(eEncodeAndSend(
		pxContext,
		pxContext->aeDeferredMessageTypes[0],
		pxContext->aeDeferredReasons[0],
		pxContext->aauDeferredPayloads[0],
		pxContext->axDeferredPayloadLengths[0]) == RSRX_TRANSPORT_STATUS_OK)
	{
		vDropDeferredSendFront(pxContext);
		pxContext->xOutboundTelemetry.uDeferredDispatchCount++;
	}
}

static void vClearOutstandingSend(
	rsrx_transport_adapter_context_t * pxContext,
	uint32_t uIsFeedbackClear,
	uint32_t uIsInboundClear)
{
	if(pxContext == (rsrx_transport_adapter_context_t *)0)
	{
		return;
	}

	if(pxContext->uHasOutstandingSend != 0U)
	{
		if(uIsInboundClear != 0U)
		{
			pxContext->xOutboundTelemetry.uClearOnInboundCount++;
		}
		else if(uIsFeedbackClear != 0U)
		{
			pxContext->xOutboundTelemetry.uClearOnFeedbackCount++;
		}
		else
		{
			pxContext->xOutboundTelemetry.uClearManualCount++;
		}
	}

	pxContext->xOutboundTelemetry.uLastBusyRejectEscalated = 0U;
	pxContext->xOutboundTelemetry.uConsecutiveBusyRejectedSendCount = 0U;
	pxContext->uHasOutstandingSend = 0U;
	pxContext->eLastOutstandingSendChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	vDispatchDeferredSendIfPresent(pxContext);
}

static rsrx_transport_status_t eEncodeAndSend(
	rsrx_transport_adapter_context_t * pxContext,
	rsrx_message_type_t eMessageType,
	rsrx_reason_code_t eReason,
	const uint8_t * puPayload,
	size_t xPayloadLength)
{
	rsrx_transport_send_request_t xRequest;
	rsrx_encode_request_t xEncodeRequest;
	rsrx_encode_buffer_t xEncodeBuffer;
	rsrx_transport_channel_id_t eSelectedChannelId;
	rsrx_transport_status_t eSendStatus;

	if((pxContext == (rsrx_transport_adapter_context_t *)0) ||
		((puPayload == (const uint8_t *)0) && (xPayloadLength > 0U)))
	{
		if(pxContext != (rsrx_transport_adapter_context_t *)0)
		{
			pxContext->xOutboundTelemetry.eLastSendStatus = RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
		}
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	if(pxContext->uHasOutstandingSend != 0U)
	{
		if((eMessageType == RSRX_MESSAGE_TYPE_DATA) &&
			(pxContext->uDeferredSendCount < 2U) &&
			(xPayloadLength <= sizeof(pxContext->aauDeferredPayloads[0])))
		{
			const uint32_t uDeferredIndex = pxContext->uDeferredSendCount;
			size_t xIndex;

			for(xIndex = 0U; xIndex < xPayloadLength; ++xIndex)
			{
				pxContext->aauDeferredPayloads[uDeferredIndex][xIndex] = puPayload[xIndex];
			}
			pxContext->axDeferredPayloadLengths[uDeferredIndex] = xPayloadLength;
			pxContext->aeDeferredMessageTypes[uDeferredIndex] = eMessageType;
			pxContext->aeDeferredReasons[uDeferredIndex] = eReason;
			pxContext->uDeferredSendCount++;
			vRefreshDeferredQueueState(pxContext);
			pxContext->xOutboundTelemetry.eLastSendStatus = RSRX_TRANSPORT_STATUS_OK;
			pxContext->xOutboundTelemetry.uQueuedSendCount++;
			pxContext->xOutboundTelemetry.uConsecutiveBusyRejectedSendCount = 0U;
			return RSRX_TRANSPORT_STATUS_OK;
		}

		pxContext->xOutboundTelemetry.eLastSendStatus = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
		pxContext->xOutboundTelemetry.uBusyRejectedSendCount++;
		pxContext->xOutboundTelemetry.uQueueOverflowRejectCount++;
		pxContext->xOutboundTelemetry.uConsecutiveBusyRejectedSendCount++;
		if(pxContext->xOutboundTelemetry.uConsecutiveBusyRejectedSendCount >
			pxContext->xOutboundTelemetry.uMaxConsecutiveBusyRejectedSendCount)
		{
			pxContext->xOutboundTelemetry.uMaxConsecutiveBusyRejectedSendCount =
				pxContext->xOutboundTelemetry.uConsecutiveBusyRejectedSendCount;
		}
		return RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	}

	if(rsrx_protocol_context_build_encode_request(
		&pxContext->xProtocolContext,
		eMessageType,
		eReason,
		puPayload,
		xPayloadLength,
		&xEncodeRequest) != RSRX_STATUS_OK)
	{
		pxContext->xOutboundTelemetry.eLastSendStatus = RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	xEncodeBuffer.puBuffer = pxContext->auEncodedFrame;
	xEncodeBuffer.xBufferCapacity = sizeof(pxContext->auEncodedFrame);
	xEncodeBuffer.xEncodedLength = 0U;

	if(pxContext->xCodecPort.pfEncode(&xEncodeRequest, &xEncodeBuffer) != RSRX_CODEC_STATUS_OK)
	{
		pxContext->xOutboundTelemetry.eLastSendStatus = RSRX_TRANSPORT_STATUS_TX_ERROR;
		return RSRX_TRANSPORT_STATUS_TX_ERROR;
	}

	if(pxContext->pxChannelManager != (rsrx_channel_manager_context_t *)0)
	{
		eSelectedChannelId = rsrx_channel_manager_get_active_channel(
			pxContext->pxChannelManager);
		xRequest.eChannelId = (eSelectedChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) ?
			eSelectedChannelId :
			pxContext->eDefaultChannelId;
	}
	else
	{
		xRequest.eChannelId = pxContext->eDefaultChannelId;
	}
	xRequest.puPayload = pxContext->auEncodedFrame;
	xRequest.xPayloadLength = xEncodeBuffer.xEncodedLength;
	xRequest.eReason = eReason;

	eSendStatus = pxContext->xTransportPort.pfSend(
		pxContext->xTransportPort.pvContext,
		&xRequest);
	if(eSendStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		pxContext->xOutboundTelemetry.eLastSendStatus = eSendStatus;
		return eSendStatus;
	}

	pxContext->eLastOutstandingSendChannelId = xRequest.eChannelId;
	pxContext->uHasOutstandingSend = 1U;
	pxContext->xOutboundTelemetry.eLastSendStatus = RSRX_TRANSPORT_STATUS_OK;
	pxContext->xOutboundTelemetry.uAcceptedSendCount++;
	pxContext->xOutboundTelemetry.uConsecutiveBusyRejectedSendCount = 0U;

	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t eRefreshChannelManagerState(
	rsrx_transport_adapter_context_t * pxContext)
{
	rsrx_transport_channel_state_t xProbeState;
	uint32_t uIndex;

	if((pxContext == (rsrx_transport_adapter_context_t *)0) ||
		(pxContext->pxChannelManager == (rsrx_channel_manager_context_t *)0))
	{
		return RSRX_TRANSPORT_STATUS_OK;
	}

	for(uIndex = 0U; uIndex < pxContext->pxChannelManager->xConfig.uChannelCount; ++uIndex)
	{
		xProbeState.eChannelId =
			pxContext->pxChannelManager->xConfig.axChannels[uIndex].eChannelId;
		xProbeState.uIsAvailable = 0U;
		if(pxContext->xTransportPort.pfQueryChannel(
			pxContext->xTransportPort.pvContext,
			&xProbeState) != RSRX_TRANSPORT_STATUS_OK)
		{
			return RSRX_TRANSPORT_STATUS_RX_ERROR;
		}
		(void)rsrx_channel_manager_update_channel(
			pxContext->pxChannelManager,
			uIndex,
			&xProbeState);
	}

	return RSRX_TRANSPORT_STATUS_OK;
}

rsrx_transport_status_t rsrx_transport_adapter_init(
	rsrx_transport_adapter_context_t * pxContext,
	const rsrx_transport_port_t * pxTransportPort,
	const rsrx_codec_port_t * pxCodecPort,
	rsrx_channel_manager_context_t * pxChannelManager,
	rsrx_transport_channel_id_t eDefaultChannelId,
	const uint8_t * puFramePayload,
	size_t xFramePayloadLength)
{
	if((pxContext == (rsrx_transport_adapter_context_t *)0) ||
		(uTransportPortIsValid(pxTransportPort) == 0U) ||
		(uCodecPortIsValid(pxCodecPort) == 0U))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xTransportPort = *pxTransportPort;
	pxContext->xCodecPort = *pxCodecPort;
	pxContext->pxChannelManager = pxChannelManager;
	(void)rsrx_protocol_context_init(&pxContext->xProtocolContext);
	pxContext->uHasLastInboundMessage = 0U;
	pxContext->eLastOutstandingSendChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxContext->uHasOutstandingSend = 0U;
	pxContext->aeDeferredMessageTypes[0] = RSRX_MESSAGE_TYPE_INVALID;
	pxContext->aeDeferredMessageTypes[1] = RSRX_MESSAGE_TYPE_INVALID;
	pxContext->aeDeferredReasons[0] = RSRX_REASON_NONE;
	pxContext->aeDeferredReasons[1] = RSRX_REASON_NONE;
	pxContext->axDeferredPayloadLengths[0] = 0U;
	pxContext->axDeferredPayloadLengths[1] = 0U;
	pxContext->uDeferredSendCount = 0U;
	pxContext->uHasDeferredSend = 0U;
	pxContext->xOutboundTelemetry.eLastSendStatus = RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	pxContext->xOutboundTelemetry.uAcceptedSendCount = 0U;
	pxContext->xOutboundTelemetry.uQueuedSendCount = 0U;
	pxContext->xOutboundTelemetry.uDeferredDispatchCount = 0U;
	pxContext->xOutboundTelemetry.uQueueOverflowRejectCount = 0U;
	pxContext->xOutboundTelemetry.uBusyRejectedSendCount = 0U;
	pxContext->xOutboundTelemetry.uConsecutiveBusyRejectedSendCount = 0U;
	pxContext->xOutboundTelemetry.uMaxConsecutiveBusyRejectedSendCount = 0U;
	pxContext->xOutboundTelemetry.uBusyRejectEscalationCount = 0U;
	pxContext->xOutboundTelemetry.uLastBusyRejectEscalated = 0U;
	pxContext->xOutboundTelemetry.uClearOnInboundCount = 0U;
	pxContext->xOutboundTelemetry.uClearOnFeedbackCount = 0U;
	pxContext->xOutboundTelemetry.uClearManualCount = 0U;
	pxContext->eDefaultChannelId = eDefaultChannelId;
	pxContext->puFramePayload = puFramePayload;
	pxContext->xFramePayloadLength = xFramePayloadLength;

	return RSRX_TRANSPORT_STATUS_OK;
}

void rsrx_transport_executor_dispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	rsrx_transport_adapter_context_t * pxContext =
		(rsrx_transport_adapter_context_t *)pvContext;
	rsrx_message_type_t eMessageType;
	size_t xPayloadLength;
	const uint8_t * puPayload;
	(void)uActionIndex;

	if((pxContext == (rsrx_transport_adapter_context_t *)0) ||
		(pxTransition == (const rsrx_transition_result_t *)0) ||
		(uActionUsesTransport(eAction) == 0U))
	{
		return;
	}

	eMessageType = eMapActionToMessageType(eAction);
	if(eMessageType == RSRX_MESSAGE_TYPE_INVALID)
	{
		return;
	}

	puPayload = puResolvePayload(pxContext, eAction, &xPayloadLength);
	(void)eEncodeAndSend(
		pxContext,
		eMessageType,
		pxTransition->eReason,
		puPayload,
		xPayloadLength);
}

void rsrx_transport_adapter_record_inbound_message(
	rsrx_transport_adapter_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage)
{
	if((pxContext == (rsrx_transport_adapter_context_t *)0) ||
		(pxMessage == (const rsrx_decoded_message_t *)0))
	{
		return;
	}

	(void)rsrx_protocol_context_record_inbound_message(
		&pxContext->xProtocolContext,
		pxMessage);
	vClearOutstandingSend(pxContext, 0U, 1U);
	pxContext->xLastInboundMessage = *pxMessage;
	pxContext->uHasLastInboundMessage = 1U;
}

const rsrx_decoded_message_t * rsrx_transport_adapter_get_last_inbound_message(
	const rsrx_transport_adapter_context_t * pxContext)
{
	if((pxContext == (const rsrx_transport_adapter_context_t *)0) ||
		(pxContext->uHasLastInboundMessage == 0U))
	{
		return (const rsrx_decoded_message_t *)0;
	}

	return &pxContext->xLastInboundMessage;
}

rsrx_transport_status_t rsrx_transport_adapter_query_channel(
	const rsrx_transport_adapter_context_t * pxContext,
	rsrx_transport_channel_state_t * pxState)
{
	rsrx_channel_selection_result_t xSelection;

	if((pxContext == (const rsrx_transport_adapter_context_t *)0) ||
		(pxState == (rsrx_transport_channel_state_t *)0))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	if(pxContext->pxChannelManager == (rsrx_channel_manager_context_t *)0)
	{
		pxState->eChannelId = pxContext->eDefaultChannelId;
		pxState->uIsAvailable = 0U;

		return pxContext->xTransportPort.pfQueryChannel(
			pxContext->xTransportPort.pvContext,
			pxState);
	}

	if(eRefreshChannelManagerState((rsrx_transport_adapter_context_t *)pxContext) !=
		RSRX_TRANSPORT_STATUS_OK)
	{
		return RSRX_TRANSPORT_STATUS_RX_ERROR;
	}

	if(rsrx_channel_manager_select_channel(
		pxContext->pxChannelManager,
		&xSelection) != RSRX_CHANNEL_MANAGER_STATUS_OK)
	{
		pxState->eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
		pxState->uIsAvailable = 0U;
		return RSRX_TRANSPORT_STATUS_CHANNEL_DOWN;
	}

	pxState->eChannelId = xSelection.eSelectedChannelId;
	pxState->uIsAvailable = (uint32_t)(xSelection.uAvailableChannelCount > 0U);
	return RSRX_TRANSPORT_STATUS_OK;
}

rsrx_transport_status_t rsrx_transport_adapter_receive_frame(
	const rsrx_transport_adapter_context_t * pxContext,
	rsrx_transport_frame_t * pxFrame)
{
	if((pxContext == (const rsrx_transport_adapter_context_t *)0) ||
		(pxFrame == (rsrx_transport_frame_t *)0))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	return pxContext->xTransportPort.pfReceive(
		pxContext->xTransportPort.pvContext,
		pxFrame);
}

rsrx_transport_status_t rsrx_transport_adapter_send_application_data(
	rsrx_transport_adapter_context_t * pxContext,
	const uint8_t * puPayload,
	size_t xPayloadLength)
{
	return eEncodeAndSend(
		pxContext,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_APPLICATION_DATA_REQUESTED,
		puPayload,
		xPayloadLength);
}

uint32_t rsrx_transport_adapter_has_outstanding_send(
	const rsrx_transport_adapter_context_t * pxContext)
{
	if(pxContext == (const rsrx_transport_adapter_context_t *)0)
	{
		return 0U;
	}

	return pxContext->uHasOutstandingSend;
}

rsrx_transport_channel_id_t rsrx_transport_adapter_get_outstanding_send_channel(
	const rsrx_transport_adapter_context_t * pxContext)
{
	if((pxContext == (const rsrx_transport_adapter_context_t *)0) ||
		(pxContext->uHasOutstandingSend == 0U))
	{
		return RSRX_TRANSPORT_CHANNEL_INVALID;
	}

	return pxContext->eLastOutstandingSendChannelId;
}

void rsrx_transport_adapter_clear_outstanding_send(
	rsrx_transport_adapter_context_t * pxContext)
{
	vClearOutstandingSend(pxContext, 0U, 0U);
}

void rsrx_transport_adapter_clear_outstanding_send_on_feedback(
	rsrx_transport_adapter_context_t * pxContext)
{
	vClearOutstandingSend(pxContext, 1U, 0U);
}

const rsrx_outbound_send_telemetry_t * rsrx_transport_adapter_get_outbound_telemetry(
	const rsrx_transport_adapter_context_t * pxContext)
{
	if(pxContext == (const rsrx_transport_adapter_context_t *)0)
	{
		return (const rsrx_outbound_send_telemetry_t *)0;
	}

	return &pxContext->xOutboundTelemetry;
}

void rsrx_transport_adapter_note_busy_reject_escalation(
	rsrx_transport_adapter_context_t * pxContext)
{
	if(pxContext == (rsrx_transport_adapter_context_t *)0)
	{
		return;
	}

	pxContext->xOutboundTelemetry.uBusyRejectEscalationCount++;
	pxContext->xOutboundTelemetry.uLastBusyRejectEscalated = 1U;
}

void rsrx_transport_adapter_clear_retransmission_context(
	rsrx_transport_adapter_context_t * pxContext)
{
	if(pxContext == (rsrx_transport_adapter_context_t *)0)
	{
		return;
	}

	(void)rsrx_protocol_context_clear_retransmission(
		&pxContext->xProtocolContext);
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
	rsrx_transport_adapter_context_t * pxTransportContext,
	rsrx_platform_adapter_context_t * pxPlatformContext,
	const rsrx_action_executor_t * pxApplicationExecutor,
	const rsrx_action_executor_t * pxApiExecutor,
	const rsrx_action_executor_t * pxLifecycleExecutor)
{
	if((pxExecutors == (rsrx_action_executor_table_t *)0) ||
		(pxTransportContext == (rsrx_transport_adapter_context_t *)0) ||
		(uTransportPortIsValid(&pxTransportContext->xTransportPort) == 0U) ||
		(uCodecPortIsValid(&pxTransportContext->xCodecPort) == 0U) ||
		(pxPlatformContext == (rsrx_platform_adapter_context_t *)0) ||
		(uPortTableIsValid(&pxPlatformContext->xPlatformPorts) == 0U) ||
		(uExecutorIsValid(pxApplicationExecutor) == 0U) ||
		(uExecutorIsValid(pxApiExecutor) == 0U) ||
		(uExecutorIsValid(pxLifecycleExecutor) == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxExecutors->xTransportExecutor.pvContext = pxTransportContext;
	pxExecutors->xTransportExecutor.pfDispatch = rsrx_transport_executor_dispatch;
	pxExecutors->xTimerExecutor.pvContext = pxPlatformContext;
	pxExecutors->xTimerExecutor.pfDispatch = rsrx_platform_timer_executor_dispatch;
	pxExecutors->xApplicationExecutor = *pxApplicationExecutor;
	pxExecutors->xApiExecutor = *pxApiExecutor;
	pxExecutors->xDiagnosticsExecutor.pvContext = pxPlatformContext;
	pxExecutors->xDiagnosticsExecutor.pfDispatch = rsrx_platform_diagnostics_executor_dispatch;
	pxExecutors->xLifecycleExecutor = *pxLifecycleExecutor;

	return RSRX_STATUS_OK;
}
