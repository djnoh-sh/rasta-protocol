#include <stdio.h>
#include <stdlib.h>

#include "rsrx_api.h"
#include "rsrx_codec.h"
#include "rsrx_transport_supervisor.h"

typedef struct
{
	rsrx_transport_send_request_t xLastRequest;
	rsrx_transport_frame_t axReceiveFrames[4];
	rsrx_transport_status_t aeReceiveStatuses[4];
	uint32_t uReceiveScriptCount;
	uint32_t uReceiveScriptIndex;
	uint32_t uSendCount;
	uint32_t uReceiveCount;
	uint32_t uQueryCount;
	uint32_t uChannelAvailable;
} test_transport_context_t;

typedef struct
{
	rsrx_monotonic_time_ns_t uNowNs;
} test_clock_context_t;

typedef struct
{
	rsrx_timer_command_t xLastCommand;
	uint32_t uCallCount;
} test_timer_context_t;

typedef struct
{
	rsrx_diagnostic_record_t xLastRecord;
	uint32_t uCallCount;
} test_diagnostics_context_t;

typedef struct
{
	rsrx_application_data_indication_t xLastIndication;
	uint32_t uCallCount;
} test_application_context_t;

typedef struct
{
	uint32_t uCallCount;
} test_counter_t;

static void vAssertTrue(int iCondition, const char * pcMessage)
{
	if(iCondition == 0)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s\n", pcMessage);
		exit(EXIT_FAILURE);
	}
}

static rsrx_transport_status_t eTransportSend(void * pvContext, const rsrx_transport_send_request_t * pxRequest)
{
	test_transport_context_t * pxContext = (test_transport_context_t *)pvContext;
	pxContext->xLastRequest = *pxRequest;
	pxContext->uSendCount++;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t eTransportReceive(void * pvContext, rsrx_transport_frame_t * pxFrame)
{
	test_transport_context_t * pxContext = (test_transport_context_t *)pvContext;

	if((pxContext == (test_transport_context_t *)0) ||
		(pxFrame == (rsrx_transport_frame_t *)0))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uReceiveCount++;
	if(pxContext->uReceiveScriptIndex < pxContext->uReceiveScriptCount)
	{
		*pxFrame = pxContext->axReceiveFrames[pxContext->uReceiveScriptIndex];
		pxContext->uReceiveScriptIndex++;
		return pxContext->aeReceiveStatuses[pxContext->uReceiveScriptIndex - 1U];
	}

	return RSRX_TRANSPORT_STATUS_UNAVAILABLE;
}

static rsrx_transport_status_t eTransportQuery(void * pvContext, rsrx_transport_channel_state_t * pxState)
{
	test_transport_context_t * pxContext = (test_transport_context_t *)pvContext;

	if(pxState != (rsrx_transport_channel_state_t *)0)
	{
		pxState->eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
		pxState->uIsAvailable = pxContext->uChannelAvailable;
	}

	pxContext->uQueryCount++;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_platform_status_t eClockNow(void * pvContext, rsrx_monotonic_time_ns_t * puNowNs)
{
	test_clock_context_t * pxContext = (test_clock_context_t *)pvContext;
	*puNowNs = pxContext->uNowNs;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t eTimerCommand(void * pvContext, const rsrx_timer_command_t * pxCommand)
{
	test_timer_context_t * pxContext = (test_timer_context_t *)pvContext;
	pxContext->xLastCommand = *pxCommand;
	pxContext->uCallCount++;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t eDiagnosticWrite(void * pvContext, const rsrx_diagnostic_record_t * pxRecord)
{
	test_diagnostics_context_t * pxContext = (test_diagnostics_context_t *)pvContext;
	pxContext->xLastRecord = *pxRecord;
	pxContext->uCallCount++;
	return RSRX_PLATFORM_STATUS_OK;
}

static void vApplicationDataNotify(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	const rsrx_application_data_indication_t * pxIndication)
{
	test_application_context_t * pxContext = (test_application_context_t *)pvContext;
	(void)pxReport;
	pxContext->xLastIndication = *pxIndication;
	pxContext->uCallCount++;
}

static void vApiNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport)
{
	test_counter_t * pxContext = (test_counter_t *)pvContext;
	(void)pxReport;
	pxContext->uCallCount++;
}

static void vLifecycleNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport, rsrx_action_t eAction, uint32_t uActionIndex)
{
	test_counter_t * pxContext = (test_counter_t *)pvContext;
	(void)pxReport;
	(void)eAction;
	(void)uActionIndex;
	pxContext->uCallCount++;
}

static void vSetReceiveScript(
	test_transport_context_t * pxTransport,
	const rsrx_transport_frame_t * pxFrames,
	const rsrx_transport_status_t * peStatuses,
	uint32_t uScriptCount)
{
	uint32_t uIndex;

	pxTransport->uReceiveScriptCount = uScriptCount;
	pxTransport->uReceiveScriptIndex = 0U;
	for(uIndex = 0U; uIndex < uScriptCount; ++uIndex)
	{
		pxTransport->axReceiveFrames[uIndex] = pxFrames[uIndex];
		pxTransport->aeReceiveStatuses[uIndex] = peStatuses[uIndex];
	}
}

static void vEncodeFrame(
	rsrx_message_type_t eMessageType,
	rsrx_reason_code_t eReason,
	uint32_t uSequenceNumber,
	uint32_t uConfirmationNumber,
	const uint8_t * puPayload,
	size_t xPayloadLength,
	uint8_t * puBuffer,
	size_t xBufferCapacity,
	size_t * pxEncodedLength)
{
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.eMessageType = eMessageType;
	xRequest.eReason = eReason;
	xRequest.uSequenceNumber = uSequenceNumber;
	xRequest.uConfirmationNumber = uConfirmationNumber;
	xRequest.puPayload = puPayload;
	xRequest.xPayloadLength = xPayloadLength;
	xBuffer.puBuffer = puBuffer;
	xBuffer.xBufferCapacity = xBufferCapacity;
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "encode frame");
	*pxEncodedLength = xBuffer.xEncodedLength;
}

static void vFillConfig(
	rsrx_session_config_t * pxConfig,
	test_transport_context_t * pxTransport,
	test_clock_context_t * pxClock,
	test_timer_context_t * pxTimer,
	test_diagnostics_context_t * pxDiagnostics,
	test_application_context_t * pxApplication,
	test_counter_t * pxApiCounter,
	test_counter_t * pxLifecycleCounter,
	const uint8_t * puFramePayload,
	size_t xFramePayloadLength)
{
	pxConfig->xTransportPort.pvContext = pxTransport;
	pxConfig->xTransportPort.pfSend = eTransportSend;
	pxConfig->xTransportPort.pfReceive = eTransportReceive;
	pxConfig->xTransportPort.pfQueryChannel = eTransportQuery;
	pxConfig->xCodecPort = *rsrx_codec_get_default_port();
	pxConfig->xPlatformPorts.xClock.pvContext = pxClock;
	pxConfig->xPlatformPorts.xClock.pfNow = eClockNow;
	pxConfig->xPlatformPorts.xTimer.pvContext = pxTimer;
	pxConfig->xPlatformPorts.xTimer.pfCommand = eTimerCommand;
	pxConfig->xPlatformPorts.xDiagnostics.pvContext = pxDiagnostics;
	pxConfig->xPlatformPorts.xDiagnostics.pfWrite = eDiagnosticWrite;
	pxConfig->eDefaultChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->puFramePayload = puFramePayload;
	pxConfig->xFramePayloadLength = xFramePayloadLength;
	pxConfig->uSupervisionIntervalNs = 100U;
	pxConfig->uRetransmissionIntervalNs = 200U;
	pxConfig->uDiagnosticFlushIntervalNs = 300U;
	pxConfig->pvApplicationDataContext = pxApplication;
	pxConfig->pfApplicationData = vApplicationDataNotify;
	pxConfig->pvApiCallbackContext = pxApiCounter;
	pxConfig->pfApiNotification = vApiNotify;
	pxConfig->pvLifecycleCallbackContext = pxLifecycleCounter;
	pxConfig->pfLifecycleNotification = vLifecycleNotify;
}

static void vTestIntegratedSessionSupervisorFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t axFrames[3];
	rsrx_transport_status_t aeStatuses[3];
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auInboundDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload[3] = { 0xA1U, 0xA2U, 0xA3U };
	static const uint8_t auOutboundPayload[2] = { 0xB1U, 0xB2U };
	size_t xHandshakeLength;
	size_t xInboundDataLength;

	xTransport.uChannelAvailable = 1U;
	vFillConfig(
		&xConfig,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auFramePayload,
		sizeof(auFramePayload));

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "integration session connect");
	vAssertTrue(xTransport.uSendCount == 1U, "integration connect request sent");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_CONNECT_RESPONSE,
		RSRX_REASON_HANDSHAKE_COMPLETED,
		1U,
		1U,
		(const uint8_t *)0,
		0U,
		auHandshakeFrame,
		sizeof(auHandshakeFrame),
		&xHandshakeLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		1U,
		auInboundDataPayload,
		sizeof(auInboundDataPayload),
		auInboundDataFrame,
		sizeof(auInboundDataFrame),
		&xInboundDataLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auInboundDataFrame;
	axFrames[1].xPayloadLength = xInboundDataLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = (const uint8_t *)0;
	axFrames[2].xPayloadLength = 0U;
	axFrames[2].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 3U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 4U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "integration pump receive");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "integration established after pump");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 2U, "integration pump processed count");
	vAssertTrue(xApplication.uCallCount == 1U, "integration application callback count");
	vAssertTrue(xApplication.xLastIndication.xPayloadLength == sizeof(auInboundDataPayload), "integration inbound payload length");
	vAssertTrue(xApplication.xLastIndication.puPayload[0] == auInboundDataPayload[0], "integration inbound payload byte");
	vAssertTrue(xApplication.xLastIndication.uSequenceNumber == 2U, "integration inbound sequence");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "integration outbound data send");
	vAssertTrue(xTransport.uSendCount == 2U, "integration outbound send count");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_APPLICATION_DATA_REQUESTED, "integration outbound reason");
	vAssertTrue(xTransport.xLastRequest.xPayloadLength == (D_RSRX_CODEC_HEADER_BYTES + sizeof(auOutboundPayload)), "integration outbound length");
}

static void vTestIntegratedRetransmissionRecoveryFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t axFrames[4];
	rsrx_transport_status_t aeStatuses[4];
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0xC1U, 0xC2U };
	static const uint8_t auRecoveryPayload[2] = { 0xD1U, 0xD2U };
	size_t xHandshakeLength;
	size_t xGapLength;
	size_t xRecoveryLength;

	xTransport.uChannelAvailable = 1U;
	vFillConfig(
		&xConfig,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auFramePayload,
		sizeof(auFramePayload));

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "recovery integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "recovery integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "recovery integration session connect");
	vAssertTrue(xTransport.uSendCount == 1U, "recovery integration connect request sent");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_CONNECT_RESPONSE,
		RSRX_REASON_HANDSHAKE_COMPLETED,
		1U,
		1U,
		(const uint8_t *)0,
		0U,
		auHandshakeFrame,
		sizeof(auHandshakeFrame),
		&xHandshakeLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auGapPayload,
		sizeof(auGapPayload),
		auGapFrame,
		sizeof(auGapFrame),
		&xGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		2U,
		auRecoveryPayload,
		sizeof(auRecoveryPayload),
		auRecoveryFrame,
		sizeof(auRecoveryFrame),
		&xRecoveryLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame;
	axFrames[1].xPayloadLength = xGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = auRecoveryFrame;
	axFrames[2].xPayloadLength = xRecoveryLength;
	axFrames[2].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[3].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[3].puPayload = (const uint8_t *)0;
	axFrames[3].xPayloadLength = 0U;
	axFrames[3].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[3] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 4U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "recovery integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 6U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "recovery integration pump receive");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "recovery integration final established");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 3U, "recovery integration processed count");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "recovery integration final reason");
	vAssertTrue(xTransport.uSendCount == 2U, "recovery integration retransmission request sent");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "recovery integration retransmission reason");
	vAssertTrue(xApplication.uCallCount == 0U, "recovery integration no application callback on recovery path");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "recovery integration lifecycle callback count");
}

int main(void)
{
	vTestIntegratedSessionSupervisorFlow();
	vTestIntegratedRetransmissionRecoveryFlow();

	(void)printf("rsrx_session_supervisor_flow_test: all tests passed\n");
	return EXIT_SUCCESS;
}
