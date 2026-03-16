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
		(eAction == RSRX_ACTION_DELIVER_DATA) ||
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

		case RSRX_ACTION_DELIVER_DATA:
			return RSRX_MESSAGE_TYPE_DATA;

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
	if(pxPayloadLength == (size_t *)0)
	{
		return (const uint8_t *)0;
	}

	if(eAction == RSRX_ACTION_DELIVER_DATA)
	{
		*pxPayloadLength = pxContext->xFramePayloadLength;
		return pxContext->puFramePayload;
	}

	*pxPayloadLength = 0U;
	return (const uint8_t *)0;
}

rsrx_transport_status_t rsrx_transport_adapter_init(
	rsrx_transport_adapter_context_t * pxContext,
	const rsrx_transport_port_t * pxTransportPort,
	const rsrx_codec_port_t * pxCodecPort,
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
	rsrx_transport_send_request_t xRequest;
	rsrx_encode_request_t xEncodeRequest;
	rsrx_encode_buffer_t xEncodeBuffer;
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
	xEncodeRequest.eMessageType = eMessageType;
	xEncodeRequest.eReason = pxTransition->eReason;
	xEncodeRequest.uSequenceNumber = 0U;
	xEncodeRequest.uConfirmationNumber = 0U;
	xEncodeRequest.puPayload = puPayload;
	xEncodeRequest.xPayloadLength = xPayloadLength;
	xEncodeBuffer.puBuffer = pxContext->auEncodedFrame;
	xEncodeBuffer.xBufferCapacity = sizeof(pxContext->auEncodedFrame);
	xEncodeBuffer.xEncodedLength = 0U;

	if(pxContext->xCodecPort.pfEncode(&xEncodeRequest, &xEncodeBuffer) != RSRX_CODEC_STATUS_OK)
	{
		return;
	}

	xRequest.eChannelId = pxContext->eDefaultChannelId;
	xRequest.puPayload = pxContext->auEncodedFrame;
	xRequest.xPayloadLength = xEncodeBuffer.xEncodedLength;
	xRequest.eReason = pxTransition->eReason;

	(void)pxContext->xTransportPort.pfSend(
		pxContext->xTransportPort.pvContext,
		&xRequest);
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
	const rsrx_action_executor_t * pxApiExecutor,
	const rsrx_action_executor_t * pxLifecycleExecutor)
{
	if((pxExecutors == (rsrx_action_executor_table_t *)0) ||
		(pxTransportContext == (rsrx_transport_adapter_context_t *)0) ||
		(uTransportPortIsValid(&pxTransportContext->xTransportPort) == 0U) ||
		(uCodecPortIsValid(&pxTransportContext->xCodecPort) == 0U) ||
		(pxPlatformContext == (rsrx_platform_adapter_context_t *)0) ||
		(uPortTableIsValid(&pxPlatformContext->xPlatformPorts) == 0U) ||
		(uExecutorIsValid(pxApiExecutor) == 0U) ||
		(uExecutorIsValid(pxLifecycleExecutor) == 0U))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxExecutors->xTransportExecutor.pvContext = pxTransportContext;
	pxExecutors->xTransportExecutor.pfDispatch = rsrx_transport_executor_dispatch;
	pxExecutors->xTimerExecutor.pvContext = pxPlatformContext;
	pxExecutors->xTimerExecutor.pfDispatch = rsrx_platform_timer_executor_dispatch;
	pxExecutors->xApiExecutor = *pxApiExecutor;
	pxExecutors->xDiagnosticsExecutor.pvContext = pxPlatformContext;
	pxExecutors->xDiagnosticsExecutor.pfDispatch = rsrx_platform_diagnostics_executor_dispatch;
	pxExecutors->xLifecycleExecutor = *pxLifecycleExecutor;

	return RSRX_STATUS_OK;
}
