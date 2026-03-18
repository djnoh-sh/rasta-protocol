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
	uint32_t uPrimaryAvailable;
	uint32_t uSecondaryAvailable;
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
	rsrx_orchestrator_report_t xLastReport;
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
		if(pxState->eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY)
		{
			pxState->uIsAvailable = pxContext->uSecondaryAvailable;
		}
		else
		{
			pxState->eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
			pxState->uIsAvailable = pxContext->uPrimaryAvailable;
		}
	}

	pxContext->uQueryCount++;
	return RSRX_TRANSPORT_STATUS_OK;
}

/* cppcheck-suppress constParameterCallback */
static rsrx_platform_status_t eClockNow(void * pvContext, rsrx_monotonic_time_ns_t * puNowNs)
{
	const test_clock_context_t * pxContext = (const test_clock_context_t *)pvContext;
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
	pxContext->uCallCount++;
	if(pxReport != (const rsrx_orchestrator_report_t *)0)
	{
		pxContext->xLastReport = *pxReport;
	}
}

static void vLifecycleNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport, rsrx_action_t eAction, uint32_t uActionIndex)
{
	test_counter_t * pxContext = (test_counter_t *)pvContext;
	(void)eAction;
	(void)uActionIndex;
	pxContext->uCallCount++;
	if(pxReport != (const rsrx_orchestrator_report_t *)0)
	{
		pxContext->xLastReport = *pxReport;
	}
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
	pxConfig->xChannelManagerConfig.eMode = RSRX_REDUNDANCY_MODE_SINGLE;
	pxConfig->xChannelManagerConfig.uChannelCount = 1U;
	pxConfig->xChannelManagerConfig.uPreferredChannelIndex = 0U;
	pxConfig->xChannelManagerConfig.uPreferredRecoveryHoldoffSelections = 0U;
	pxConfig->xChannelManagerConfig.axChannels[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->xChannelManagerConfig.axChannels[0].uIsAvailable = 1U;
	pxConfig->xChannelManagerConfig.axChannels[0].uPriority = 0U;
	pxConfig->xChannelManagerConfig.axChannels[1].eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxConfig->xChannelManagerConfig.axChannels[1].uIsAvailable = 0U;
	pxConfig->xChannelManagerConfig.axChannels[1].uPriority = 0U;
	pxConfig->puFramePayload = puFramePayload;
	pxConfig->xFramePayloadLength = xFramePayloadLength;
	pxConfig->uSupervisionIntervalNs = 100U;
	pxConfig->uRetransmissionIntervalNs = 200U;
	pxConfig->uDiagnosticFlushIntervalNs = 300U;
	pxConfig->uBusyRejectErrorThreshold = 0U;
	pxConfig->pvApplicationDataContext = pxApplication;
	pxConfig->pfApplicationData = vApplicationDataNotify;
	pxConfig->pvApiCallbackContext = pxApiCounter;
	pxConfig->pfApiNotification = vApiNotify;
	pxConfig->pvLifecycleCallbackContext = pxLifecycleCounter;
	pxConfig->pfLifecycleNotification = vLifecycleNotify;
}

static void vSetActiveStandbyConfig(
	rsrx_session_config_t * pxConfig)
{
	pxConfig->xChannelManagerConfig.eMode = RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY;
	pxConfig->xChannelManagerConfig.uChannelCount = 2U;
	pxConfig->xChannelManagerConfig.uPreferredChannelIndex = 0U;
	pxConfig->xChannelManagerConfig.uPreferredRecoveryHoldoffSelections = 0U;
	pxConfig->xChannelManagerConfig.axChannels[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->xChannelManagerConfig.axChannels[0].uIsAvailable = 1U;
	pxConfig->xChannelManagerConfig.axChannels[0].uPriority = 0U;
	pxConfig->xChannelManagerConfig.axChannels[1].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	pxConfig->xChannelManagerConfig.axChannels[1].uIsAvailable = 1U;
	pxConfig->xChannelManagerConfig.axChannels[1].uPriority = 1U;
}

static void vSetActiveStandbyHoldoffConfig(
	rsrx_session_config_t * pxConfig,
	uint32_t uHoldoffSelections)
{
	vSetActiveStandbyConfig(pxConfig);
	pxConfig->xChannelManagerConfig.uPreferredRecoveryHoldoffSelections =
		uHoldoffSelections;
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
	rsrx_transport_frame_t axFrames[4];
	rsrx_transport_status_t aeStatuses[4];
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auInboundDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload[3] = { 0xA1U, 0xA2U, 0xA3U };
	static const uint8_t auOutboundPayload[2] = { 0xB1U, 0xB2U };
	size_t xHandshakeLength;
	size_t xInboundDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

static void vTestIntegratedDeferredQueueTelemetryFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xSendCompletedFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0xC1U, 0xC2U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "queue telemetry integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "queue telemetry integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "queue telemetry integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = (const uint8_t *)0;
	axFrames[1].xPayloadLength = 0U;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "queue telemetry integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 2U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "queue telemetry integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "queue telemetry integration established");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "queue telemetry integration first send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "queue telemetry integration deferred send");
	pxTelemetry = rsrx_session_get_outbound_telemetry(&xSession);
	vAssertTrue(pxTelemetry != (const rsrx_outbound_send_telemetry_t *)0, "queue telemetry integration telemetry available");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 1U, "queue telemetry integration queued count");
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xSession.xTransportAdapter) == 1U, "queue telemetry integration outstanding present");
	vAssertTrue(xSession.xTransportAdapter.uHasDeferredSend == 1U, "queue telemetry integration deferred present");

	xSendCompletedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendCompletedFrame.puPayload = auFramePayload;
	xSendCompletedFrame.xPayloadLength = sizeof(auFramePayload);
	xSendCompletedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendCompletedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "queue telemetry integration send completed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED, "queue telemetry integration decision");
	vAssertTrue(pxSupervisorReport->uOutstandingSendPresent == 1U, "queue telemetry integration report outstanding");
	vAssertTrue(pxSupervisorReport->uDeferredSendPresent == 0U, "queue telemetry integration report deferred cleared");
	vAssertTrue(pxSupervisorReport->uQueuedSendCount == 1U, "queue telemetry integration report queued");
	vAssertTrue(pxSupervisorReport->uDeferredDispatchCount == 1U, "queue telemetry integration report dispatched");
	vAssertTrue(pxSupervisorReport->uQueueOverflowRejectCount == 0U, "queue telemetry integration report no overflow");
}

static void vTestIntegratedQueueOverflowRejectFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0xD1U, 0xD2U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "queue overflow integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "queue overflow integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "queue overflow integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = (const uint8_t *)0;
	axFrames[1].xPayloadLength = 0U;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "queue overflow integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 2U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "queue overflow integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "queue overflow integration established");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "queue overflow integration first send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "queue overflow integration deferred send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_REJECTED, "queue overflow integration overflow reject");

	pxTelemetry = rsrx_session_get_outbound_telemetry(&xSession);
	vAssertTrue(pxTelemetry != (const rsrx_outbound_send_telemetry_t *)0, "queue overflow integration telemetry available");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 1U, "queue overflow integration queued once");
	vAssertTrue(pxTelemetry->uQueueOverflowRejectCount == 1U, "queue overflow integration overflow count");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 1U, "queue overflow integration busy reject count");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "queue overflow integration busy streak");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 0U, "queue overflow integration no escalation");
	vAssertTrue(xDiagnostics.uCallCount >= 1U, "queue overflow integration diagnostics called");
	vAssertTrue(xDiagnostics.xLastRecord.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "queue overflow integration warning diagnostic");
	vAssertTrue(xDiagnostics.xLastRecord.eSeverity == RSRX_LOG_SEVERITY_WARNING, "queue overflow integration warning severity");
	vAssertTrue(xApiCounter.uCallCount >= 1U, "queue overflow integration api callback");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "queue overflow integration api diagnostic");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eReason == RSRX_REASON_APPLICATION_DATA_REQUESTED, "queue overflow integration api reason");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eStatus == RSRX_STATUS_REJECTED, "queue overflow integration api status");
	vAssertTrue(pxSupervisorReport->uQueueOverflowRejectCount == 0U, "queue overflow integration supervisor unchanged before feedback");
}

static void vTestIntegratedBusyRejectThresholdEscalationFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0xE1U, 0xE2U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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
	xConfig.uBusyRejectErrorThreshold = 2U;

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "busy threshold integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "busy threshold integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "busy threshold integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = (const uint8_t *)0;
	axFrames[1].xPayloadLength = 0U;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "busy threshold integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 2U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "busy threshold integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "busy threshold integration established");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "busy threshold integration first send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "busy threshold integration deferred send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_REJECTED, "busy threshold integration first reject");

	pxTelemetry = rsrx_session_get_outbound_telemetry(&xSession);
	vAssertTrue(pxTelemetry != (const rsrx_outbound_send_telemetry_t *)0, "busy threshold integration telemetry available");
	vAssertTrue(xDiagnostics.xLastRecord.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "busy threshold integration first warning");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "busy threshold integration first api warning");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "busy threshold integration first streak");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 0U, "busy threshold integration no escalation before threshold");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 0U, "busy threshold integration latch clear before threshold");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_REJECTED, "busy threshold integration second reject");
	vAssertTrue(xDiagnostics.xLastRecord.eDiagnostic == RSRX_DIAG_ERROR_INTERFACE, "busy threshold integration second error");
	vAssertTrue(xDiagnostics.xLastRecord.eSeverity == RSRX_LOG_SEVERITY_ERROR, "busy threshold integration second severity");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eDiagnostic == RSRX_DIAG_ERROR_INTERFACE, "busy threshold integration second api error");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eStatus == RSRX_STATUS_REJECTED, "busy threshold integration second api rejected");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 2U, "busy threshold integration second streak");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 1U, "busy threshold integration escalation count");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 1U, "busy threshold integration escalation latch");
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

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

static void vTestIntegratedUnconfirmedRecoveryProtocolErrorFlow(void)
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
	uint8_t auUnconfirmedRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0xE1U, 0xE2U };
	static const uint8_t auRecoveryPayload[2] = { 0xF1U, 0xF2U };
	size_t xHandshakeLength;
	size_t xGapLength;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "unconfirmed recovery integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "unconfirmed recovery integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "unconfirmed recovery integration session connect");
	vAssertTrue(xTransport.uSendCount == 1U, "unconfirmed recovery integration connect request sent");

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
		0U,
		auRecoveryPayload,
		sizeof(auRecoveryPayload),
		auUnconfirmedRecoveryFrame,
		sizeof(auUnconfirmedRecoveryFrame),
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
	axFrames[2].puPayload = auUnconfirmedRecoveryFrame;
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

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "unconfirmed recovery integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 6U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "unconfirmed recovery integration pump receive");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "unconfirmed recovery integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 3U, "unconfirmed recovery integration processed count");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "unconfirmed recovery integration protocol error event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "unconfirmed recovery integration rejected status");
	vAssertTrue(xTransport.uSendCount == 2U, "unconfirmed recovery integration retransmission request sent");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "unconfirmed recovery integration retransmission reason");
	vAssertTrue(xApplication.uCallCount == 0U, "unconfirmed recovery integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "unconfirmed recovery integration lifecycle callback count");
}

static void vTestIntegratedRepeatedGapRetransmissionFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xSendCompletedFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame1[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame2[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload1[2] = { 0xC1U, 0xC2U };
	static const uint8_t auGapPayload2[2] = { 0xC3U, 0xC4U };
	size_t xHandshakeLength;
	size_t xGapLength1;
	size_t xGapLength2;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "repeated gap integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "repeated gap integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "repeated gap integration session connect");
	vAssertTrue(xTransport.uSendCount == 1U, "repeated gap integration connect request sent");

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
		auGapPayload1,
		sizeof(auGapPayload1),
		auGapFrame1,
		sizeof(auGapFrame1),
		&xGapLength1);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auGapPayload2,
		sizeof(auGapPayload2),
		auGapFrame2,
		sizeof(auGapFrame2),
		&xGapLength2);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame1;
	axFrames[1].xPayloadLength = xGapLength1;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "repeated gap integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "repeated gap integration initial pump receive");
	vAssertTrue(xTransport.uSendCount == 2U, "repeated gap integration first retransmission send");

	xSendCompletedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendCompletedFrame.puPayload = (const uint8_t *)0;
	xSendCompletedFrame.xPayloadLength = 0U;
	xSendCompletedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendCompletedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "repeated gap integration clear outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auGapFrame2;
	axFrames[0].xPayloadLength = xGapLength2;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "repeated gap integration second gap poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "repeated gap integration retransmission pending");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "repeated gap integration effective gap event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_OK, "repeated gap integration accepted status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "repeated gap integration reason");
	vAssertTrue(xApplication.uCallCount == 0U, "repeated gap integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "repeated gap integration no lifecycle callback");
}

static void vTestIntegratedRepeatedGapRecoveryFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xSendCompletedFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame1[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame2[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload1[2] = { 0xD1U, 0xD2U };
	static const uint8_t auGapPayload2[2] = { 0xD3U, 0xD4U };
	static const uint8_t auRecoveryPayload[2] = { 0xD5U, 0xD6U };
	size_t xHandshakeLength;
	size_t xGapLength1;
	size_t xGapLength2;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "repeated gap recovery integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "repeated gap recovery integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "repeated gap recovery integration session connect");
	vAssertTrue(xTransport.uSendCount == 1U, "repeated gap recovery integration connect request sent");

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
		auGapPayload1,
		sizeof(auGapPayload1),
		auGapFrame1,
		sizeof(auGapFrame1),
		&xGapLength1);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auGapPayload2,
		sizeof(auGapPayload2),
		auGapFrame2,
		sizeof(auGapFrame2),
		&xGapLength2);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		3U,
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
	axFrames[1].puPayload = auGapFrame1;
	axFrames[1].xPayloadLength = xGapLength1;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "repeated gap recovery integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "repeated gap recovery integration first pump receive");
	vAssertTrue(xTransport.uSendCount == 2U, "repeated gap recovery integration first retransmission send");

	xSendCompletedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendCompletedFrame.puPayload = (const uint8_t *)0;
	xSendCompletedFrame.xPayloadLength = 0U;
	xSendCompletedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendCompletedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "repeated gap recovery integration first clear outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auGapFrame2;
	axFrames[0].xPayloadLength = xGapLength2;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "repeated gap recovery integration second gap poll");
	vAssertTrue(xTransport.uSendCount == 3U, "repeated gap recovery integration second retransmission send");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendCompletedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "repeated gap recovery integration second clear outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auRecoveryFrame;
	axFrames[0].xPayloadLength = xRecoveryLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "repeated gap recovery integration recovery poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "repeated gap recovery integration established");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_RECOVERY_SUCCESS, "repeated gap recovery integration effective recovery event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_OK, "repeated gap recovery integration accepted status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "repeated gap recovery integration reason");
	vAssertTrue(xTimer.xLastCommand.eTimerId == RSRX_TIMER_ID_SUPERVISION, "repeated gap recovery integration supervision timer command");
	vAssertTrue(xApplication.uCallCount == 0U, "repeated gap recovery integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "repeated gap recovery integration lifecycle callback count");
}

static void vTestIntegratedRetransmissionTimeoutFailSafeFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0xE7U, 0xE8U };
	size_t xHandshakeLength;
	size_t xGapLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retransmission timeout integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retransmission timeout integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retransmission timeout integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame;
	axFrames[1].xPayloadLength = xGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retransmission timeout integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retransmission timeout integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retransmission timeout integration retransmission pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retransmission timeout integration retransmission request sent");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_RETRANSMISSION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retransmission timeout integration timer expiry");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retransmission timeout integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "retransmission timeout integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RETRANSMISSION_FAILED, "retransmission timeout integration reason");
	vAssertTrue(xApplication.uCallCount == 0U, "retransmission timeout integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retransmission timeout integration lifecycle callback");
}

static void vTestIntegratedRetransmissionFailoverRecoveryFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xChannelDownFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0x91U, 0x92U };
	static const uint8_t auRecoveryPayload[2] = { 0x93U, 0x94U };
	size_t xHandshakeLength;
	size_t xGapLength;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans failover recovery integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover recovery integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover recovery integration session connect");

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
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover recovery integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover recovery integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover recovery integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans failover recovery integration retrans request sent");

	// cppcheck-suppress redundantAssignment
	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover recovery integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans failover recovery integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover recovery integration switch count");

	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover recovery integration clear outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auRecoveryFrame;
	axFrames[0].xPayloadLength = xRecoveryLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover recovery integration recovery poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "retrans failover recovery integration established");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_RECOVERY_SUCCESS, "retrans failover recovery integration recovery event");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "retrans failover recovery integration reason");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans failover recovery integration lifecycle callback");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans failover recovery integration no application callback");
}

static void vTestIntegratedRetransmissionChannelUpHoldoffRecoveryFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0x95U, 0x96U };
	static const uint8_t auRecoveryPayload[2] = { 0x97U, 0x98U };
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	size_t xHandshakeLength;
	size_t xGapLength;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans channel up holdoff recovery integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans channel up holdoff recovery integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans channel up holdoff recovery integration session connect");

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
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff recovery integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff recovery integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans channel up holdoff recovery integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans channel up holdoff recovery integration retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff recovery integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans channel up holdoff recovery integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans channel up holdoff recovery integration first switch count");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff recovery integration clear outstanding");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff recovery integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans channel up holdoff recovery integration hold secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans channel up holdoff recovery integration switch count held");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auRecoveryFrame;
	axFrames[0].xPayloadLength = xRecoveryLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff recovery integration recovery poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "retrans channel up holdoff recovery integration established");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_RECOVERY_SUCCESS, "retrans channel up holdoff recovery integration recovery event");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "retrans channel up holdoff recovery integration recovery reason");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff recovery integration second refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "retrans channel up holdoff recovery integration final primary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "retrans channel up holdoff recovery integration final switch count");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "retrans channel up holdoff recovery integration state retained");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans channel up holdoff recovery integration lifecycle callback");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans channel up holdoff recovery integration no application callback");
}

static void vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapRecoveryFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auFirstGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auFirstGapPayload[2] = { 0xA1U, 0xA2U };
	static const uint8_t auSecondGapPayload[2] = { 0xA3U, 0xA4U };
	static const uint8_t auRecoveryPayload[2] = { 0xA5U, 0xA6U };
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	size_t xHandshakeLength;
	size_t xFirstGapLength;
	size_t xSecondGapLength;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans channel up holdoff repeated gap recovery integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans channel up holdoff repeated gap recovery integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans channel up holdoff repeated gap recovery integration session connect");

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
		auFirstGapPayload,
		sizeof(auFirstGapPayload),
		auFirstGapFrame,
		sizeof(auFirstGapFrame),
		&xFirstGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auSecondGapPayload,
		sizeof(auSecondGapPayload),
		auSecondGapFrame,
		sizeof(auSecondGapFrame),
		&xSecondGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		3U,
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
	axFrames[1].puPayload = auFirstGapFrame;
	axFrames[1].xPayloadLength = xFirstGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff repeated gap recovery integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff repeated gap recovery integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans channel up holdoff repeated gap recovery integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans channel up holdoff repeated gap recovery integration first retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff repeated gap recovery integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans channel up holdoff repeated gap recovery integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans channel up holdoff repeated gap recovery integration first switch count");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff repeated gap recovery integration clear first outstanding");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff repeated gap recovery integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans channel up holdoff repeated gap recovery integration hold secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans channel up holdoff repeated gap recovery integration switch count held");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auSecondGapFrame;
	axFrames[0].xPayloadLength = xSecondGapLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff repeated gap recovery integration second gap poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans channel up holdoff repeated gap recovery integration still retrans pending");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "retrans channel up holdoff repeated gap recovery integration second gap event");
	vAssertTrue(xTransport.uSendCount == 3U, "retrans channel up holdoff repeated gap recovery integration second retrans request sent");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "retrans channel up holdoff repeated gap recovery integration second gap reason");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff repeated gap recovery integration clear second outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auRecoveryFrame;
	axFrames[0].xPayloadLength = xRecoveryLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff repeated gap recovery integration recovery poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "retrans channel up holdoff repeated gap recovery integration established");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_RECOVERY_SUCCESS, "retrans channel up holdoff repeated gap recovery integration recovery event");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "retrans channel up holdoff repeated gap recovery integration recovery reason");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff repeated gap recovery integration second refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "retrans channel up holdoff repeated gap recovery integration final primary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "retrans channel up holdoff repeated gap recovery integration final switch count");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "retrans channel up holdoff repeated gap recovery integration state retained");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans channel up holdoff repeated gap recovery integration lifecycle callback");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans channel up holdoff repeated gap recovery integration no application callback");
}

static void vTestIntegratedRetransmissionChannelUpHoldoffTimeoutFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0x99U, 0x9AU };
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	size_t xHandshakeLength;
	size_t xGapLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans channel up holdoff timeout integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans channel up holdoff timeout integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans channel up holdoff timeout integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame;
	axFrames[1].xPayloadLength = xGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff timeout integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff timeout integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans channel up holdoff timeout integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans channel up holdoff timeout integration retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff timeout integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans channel up holdoff timeout integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans channel up holdoff timeout integration first switch count");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff timeout integration clear outstanding");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans channel up holdoff timeout integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans channel up holdoff timeout integration hold secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans channel up holdoff timeout integration switch count held");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_RETRANSMISSION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans channel up holdoff timeout integration timer expiry");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retrans channel up holdoff timeout integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "retrans channel up holdoff timeout integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RETRANSMISSION_FAILED, "retrans channel up holdoff timeout integration reason");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans channel up holdoff timeout integration final switch count");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans channel up holdoff timeout integration lifecycle callback");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans channel up holdoff timeout integration no application callback");
}

static void vTestIntegratedRetransmissionFailoverTimeoutFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame = { RSRX_TRANSPORT_CHANNEL_PRIMARY, (const uint8_t *)0, 0U, RSRX_TRANSPORT_EVENT_NONE };
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0x95U, 0x96U };
	size_t xHandshakeLength;
	size_t xGapLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans failover timeout integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover timeout integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover timeout integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame;
	axFrames[1].xPayloadLength = xGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover timeout integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover timeout integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover timeout integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans failover timeout integration retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover timeout integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans failover timeout integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover timeout integration switch count");

	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover timeout integration clear outstanding");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_RETRANSMISSION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover timeout integration timer expiry");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retrans failover timeout integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "retrans failover timeout integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RETRANSMISSION_FAILED, "retrans failover timeout integration reason");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover timeout integration switch count retained");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans failover timeout integration lifecycle callback");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans failover timeout integration no application callback");
}

static void vTestIntegratedRetransmissionFailoverRepeatedGapRecoveryFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame = { RSRX_TRANSPORT_CHANNEL_PRIMARY, (const uint8_t *)0, 0U, RSRX_TRANSPORT_EVENT_NONE };
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auFirstGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auFirstGapPayload[2] = { 0x97U, 0x98U };
	static const uint8_t auSecondGapPayload[2] = { 0x99U, 0x9AU };
	static const uint8_t auRecoveryPayload[2] = { 0x9BU, 0x9CU };
	size_t xHandshakeLength;
	size_t xFirstGapLength;
	size_t xSecondGapLength;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans failover repeated gap integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover repeated gap integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover repeated gap integration session connect");

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
		auFirstGapPayload,
		sizeof(auFirstGapPayload),
		auFirstGapFrame,
		sizeof(auFirstGapFrame),
		&xFirstGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auSecondGapPayload,
		sizeof(auSecondGapPayload),
		auSecondGapFrame,
		sizeof(auSecondGapFrame),
		&xSecondGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		3U,
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
	axFrames[1].puPayload = auFirstGapFrame;
	axFrames[1].xPayloadLength = xFirstGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover repeated gap integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans failover repeated gap integration first retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated gap integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans failover repeated gap integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover repeated gap integration switch count");

	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated gap integration clear first outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auSecondGapFrame;
	axFrames[0].xPayloadLength = xSecondGapLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap integration second gap poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover repeated gap integration still retrans pending");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "retrans failover repeated gap integration effective second gap");
	vAssertTrue(xTransport.uSendCount == 3U, "retrans failover repeated gap integration second retrans request sent");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "retrans failover repeated gap integration second gap reason");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated gap integration clear second outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auRecoveryFrame;
	axFrames[0].xPayloadLength = xRecoveryLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap integration recovery poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "retrans failover repeated gap integration established");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_RECOVERY_SUCCESS, "retrans failover repeated gap integration recovery event");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "retrans failover repeated gap integration recovery reason");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover repeated gap integration retained switch count");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans failover repeated gap integration lifecycle callback");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans failover repeated gap integration no application callback");
}

static void vTestIntegratedRetransmissionFailoverRepeatedGapTimeoutFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame = { RSRX_TRANSPORT_CHANNEL_PRIMARY, (const uint8_t *)0, 0U, RSRX_TRANSPORT_EVENT_NONE };
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auFirstGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auFirstGapPayload[2] = { 0xA1U, 0xA2U };
	static const uint8_t auSecondGapPayload[2] = { 0xA3U, 0xA4U };
	size_t xHandshakeLength;
	size_t xFirstGapLength;
	size_t xSecondGapLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans failover repeated gap timeout integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover repeated gap timeout integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover repeated gap timeout integration session connect");

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
		auFirstGapPayload,
		sizeof(auFirstGapPayload),
		auFirstGapFrame,
		sizeof(auFirstGapFrame),
		&xFirstGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auSecondGapPayload,
		sizeof(auSecondGapPayload),
		auSecondGapFrame,
		sizeof(auSecondGapFrame),
		&xSecondGapLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auFirstGapFrame;
	axFrames[1].xPayloadLength = xFirstGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap timeout integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap timeout integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover repeated gap timeout integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans failover repeated gap timeout integration first retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated gap timeout integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans failover repeated gap timeout integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover repeated gap timeout integration switch count");

	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated gap timeout integration clear first outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auSecondGapFrame;
	axFrames[0].xPayloadLength = xSecondGapLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap timeout integration second gap poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover repeated gap timeout integration still retrans pending");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "retrans failover repeated gap timeout integration second gap event");
	vAssertTrue(xTransport.uSendCount == 3U, "retrans failover repeated gap timeout integration second retrans request sent");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated gap timeout integration clear second outstanding");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_RETRANSMISSION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated gap timeout integration timer expiry");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retrans failover repeated gap timeout integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "retrans failover repeated gap timeout integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RETRANSMISSION_FAILED, "retrans failover repeated gap timeout integration reason");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover repeated gap timeout integration retained switch count");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans failover repeated gap timeout integration lifecycle callback");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans failover repeated gap timeout integration no application callback");
}

static void vTestIntegratedStaleRetransmissionProtocolErrorFlow(void)
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
	uint8_t auStaleFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0xA4U, 0xA5U };
	static const uint8_t auStalePayload[2] = { 0xB4U, 0xB5U };
	size_t xHandshakeLength;
	size_t xGapLength;
	size_t xStaleLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "stale retransmission integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "stale retransmission integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "stale retransmission integration session connect");
	vAssertTrue(xTransport.uSendCount == 1U, "stale retransmission integration connect request sent");

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
		1U,
		1U,
		auStalePayload,
		sizeof(auStalePayload),
		auStaleFrame,
		sizeof(auStaleFrame),
		&xStaleLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame;
	axFrames[1].xPayloadLength = xGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = auStaleFrame;
	axFrames[2].xPayloadLength = xStaleLength;
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

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "stale retransmission integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 6U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "stale retransmission integration pump receive");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "stale retransmission integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 3U, "stale retransmission integration processed count");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "stale retransmission integration protocol error event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "stale retransmission integration rejected status");
	vAssertTrue(xTransport.uSendCount == 2U, "stale retransmission integration retransmission request sent");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "stale retransmission integration retransmission reason");
	vAssertTrue(xApplication.uCallCount == 0U, "stale retransmission integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "stale retransmission integration lifecycle callback");
}

static void vTestIntegratedRetransmissionFailoverStaleProtocolErrorFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame = { RSRX_TRANSPORT_CHANNEL_PRIMARY, (const uint8_t *)0, 0U, RSRX_TRANSPORT_EVENT_NONE };
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auStaleFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0xA7U, 0xA8U };
	static const uint8_t auStalePayload[2] = { 0xA9U, 0xAAU };
	size_t xHandshakeLength;
	size_t xGapLength;
	size_t xStaleLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans failover stale integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover stale integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover stale integration session connect");

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
		1U,
		2U,
		auStalePayload,
		sizeof(auStalePayload),
		auStaleFrame,
		sizeof(auStaleFrame),
		&xStaleLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame;
	axFrames[1].xPayloadLength = xGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover stale integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover stale integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover stale integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans failover stale integration retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover stale integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans failover stale integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover stale integration switch count");

	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover stale integration clear outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auStaleFrame;
	axFrames[0].xPayloadLength = xStaleLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover stale integration stale poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retrans failover stale integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "retrans failover stale integration protocol error event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "retrans failover stale integration rejected status");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans failover stale integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans failover stale integration lifecycle callback");
}

static void vTestIntegratedRetransmissionFailoverUnconfirmedRecoveryProtocolErrorFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame = { RSRX_TRANSPORT_CHANNEL_PRIMARY, (const uint8_t *)0, 0U, RSRX_TRANSPORT_EVENT_NONE };
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auUnconfirmedRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auGapPayload[2] = { 0xABU, 0xACU };
	static const uint8_t auRecoveryPayload[2] = { 0xADU, 0xAEU };
	size_t xHandshakeLength;
	size_t xGapLength;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans failover unconfirmed integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover unconfirmed integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover unconfirmed integration session connect");

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
		0U,
		auRecoveryPayload,
		sizeof(auRecoveryPayload),
		auUnconfirmedRecoveryFrame,
		sizeof(auUnconfirmedRecoveryFrame),
		&xRecoveryLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auGapFrame;
	axFrames[1].xPayloadLength = xGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover unconfirmed integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover unconfirmed integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover unconfirmed integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans failover unconfirmed integration retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover unconfirmed integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans failover unconfirmed integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover unconfirmed integration switch count");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover unconfirmed integration clear outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auUnconfirmedRecoveryFrame;
	axFrames[0].xPayloadLength = xRecoveryLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover unconfirmed integration recovery poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retrans failover unconfirmed integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover unconfirmed integration switch count retained");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "retrans failover unconfirmed integration protocol error event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "retrans failover unconfirmed integration rejected status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_REJECTED, "retrans failover unconfirmed integration decision");
	vAssertTrue(xTransport.uSendCount == 3U, "retrans failover unconfirmed integration final fail-safe send");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans failover unconfirmed integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans failover unconfirmed integration lifecycle callback");
}

static void vTestIntegratedRetransmissionFailoverRepeatedGapUnconfirmedRecoveryProtocolErrorFlow(void)
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
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_transport_frame_t xTransportEventFrame = { RSRX_TRANSPORT_CHANNEL_PRIMARY, (const uint8_t *)0, 0U, RSRX_TRANSPORT_EVENT_NONE };
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auFirstGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondGapFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auUnconfirmedRecoveryFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auFirstGapPayload[2] = { 0xAFU, 0xB0U };
	static const uint8_t auSecondGapPayload[2] = { 0xB1U, 0xB2U };
	static const uint8_t auRecoveryPayload[2] = { 0xB3U, 0xB4U };
	size_t xHandshakeLength;
	size_t xFirstGapLength;
	size_t xSecondGapLength;
	size_t xRecoveryLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "retrans failover repeated unconfirmed integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover repeated unconfirmed integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "retrans failover repeated unconfirmed integration session connect");

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
		auFirstGapPayload,
		sizeof(auFirstGapPayload),
		auFirstGapFrame,
		sizeof(auFirstGapFrame),
		&xFirstGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auSecondGapPayload,
		sizeof(auSecondGapPayload),
		auSecondGapFrame,
		sizeof(auSecondGapFrame),
		&xSecondGapLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		0U,
		auRecoveryPayload,
		sizeof(auRecoveryPayload),
		auUnconfirmedRecoveryFrame,
		sizeof(auUnconfirmedRecoveryFrame),
		&xRecoveryLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auFirstGapFrame;
	axFrames[1].xPayloadLength = xFirstGapLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated unconfirmed integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated unconfirmed integration initial pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover repeated unconfirmed integration retrans pending");
	vAssertTrue(xTransport.uSendCount == 2U, "retrans failover repeated unconfirmed integration first retrans request sent");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated unconfirmed integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "retrans failover repeated unconfirmed integration active secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover repeated unconfirmed integration switch count");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated unconfirmed integration clear first outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auSecondGapFrame;
	axFrames[0].xPayloadLength = xSecondGapLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated unconfirmed integration second gap poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retrans failover repeated unconfirmed integration still retrans pending");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "retrans failover repeated unconfirmed integration second gap event");
	vAssertTrue(xTransport.uSendCount == 3U, "retrans failover repeated unconfirmed integration second retrans request sent");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "retrans failover repeated unconfirmed integration clear second outstanding");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auUnconfirmedRecoveryFrame;
	axFrames[0].xPayloadLength = xRecoveryLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "retrans failover repeated unconfirmed integration recovery poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retrans failover repeated unconfirmed integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "retrans failover repeated unconfirmed integration switch count retained");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "retrans failover repeated unconfirmed integration protocol error event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "retrans failover repeated unconfirmed integration rejected status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_REJECTED, "retrans failover repeated unconfirmed integration decision");
	vAssertTrue(xTransport.uSendCount == 4U, "retrans failover repeated unconfirmed integration final fail-safe send");
	vAssertTrue(xApplication.uCallCount == 0U, "retrans failover repeated unconfirmed integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retrans failover repeated unconfirmed integration lifecycle callback");
}

static void vTestIntegratedTimeoutFailSafeFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "timeout integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timeout integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timeout integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "timeout integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "timeout integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "timeout integration established");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_SUPERVISION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "timeout integration timer expiry");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "timeout integration safe disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_TIMEOUT_EXPIRED, "timeout integration reason");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED, "timeout integration decision");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "timeout integration lifecycle callback");
}

static void vTestIntegratedChannelDownFailSafeFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xChannelDownFrame;
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel down integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel down integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel down integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel down integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel down integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel down integration established");

	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel down integration event");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "channel down integration safe disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "channel down integration reason");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_REJECTED, "channel down integration decision");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "channel down integration lifecycle callback");
}

static void vTestIntegratedChannelFailoverFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondaryDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xChannelDownFrame;
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auSecondaryPayload[2] = { 0x66U, 0x67U };
	static const uint8_t auOutboundPayload[2] = { 0x77U, 0x78U };
	size_t xHandshakeLength;
	size_t xSecondaryDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel failover integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel failover integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel failover integration session connect");
	vAssertTrue(xTransport.uSendCount == 1U, "channel failover integration connect request sent");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel failover integration initial primary send");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel failover integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel failover integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel failover integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover integration channel down");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED, "channel failover integration decision");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel failover integration selected secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel failover integration switch count");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 1U, "channel failover integration switch occurred");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel failover integration state retained");

	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover integration stale primary send failure");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED, "channel failover integration stale send failure decision");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "channel failover integration stale send failure budget unchanged");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_NONE, "channel failover integration stale send failure budget update");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel failover integration stale send failure state retained");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "channel failover integration outbound send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel failover integration outbound on secondary");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		1U,
		auSecondaryPayload,
		sizeof(auSecondaryPayload),
		auSecondaryDataFrame,
		sizeof(auSecondaryDataFrame),
		&xSecondaryDataLength);
	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auSecondaryDataFrame;
	axFrames[0].xPayloadLength = xSecondaryDataLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[1].puPayload = (const uint8_t *)0;
	axFrames[1].xPayloadLength = 0U;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 2U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel failover integration secondary pump");
	vAssertTrue(xApplication.uCallCount == 1U, "channel failover integration application callback");
	vAssertTrue(xApplication.xLastIndication.uSequenceNumber == 2U, "channel failover integration inbound sequence");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel failover integration active channel retained");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "channel failover integration no lifecycle callback");

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "channel failover integration preferred recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel failover integration preferred recovery selected primary");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 2U, "channel failover integration recovery switch count");

	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover integration stale secondary send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "channel failover integration stale secondary budget unchanged");

	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover integration recovered primary first failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "channel failover integration recovered primary uncorrelated budget unchanged");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED, "channel failover integration recovered primary uncorrelated decision");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "channel failover integration preferred recovery send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel failover integration switched back to primary");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover integration correlated primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "channel failover integration correlated primary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel failover integration correlated primary budget channel");
}

static void vTestIntegratedChannelRecoveryHoldoffFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xChannelDownFrame;
	rsrx_transport_frame_t axFrames[1];
	rsrx_transport_status_t aeStatuses[1];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x79U, 0x7AU };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel holdoff integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel holdoff integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel holdoff integration session connect");

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
	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel holdoff integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel holdoff integration handshake pump");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel holdoff integration failover event");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel holdoff integration failover switch count");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "channel holdoff first recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel holdoff stays secondary");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel holdoff active remains secondary");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "channel holdoff first recovery send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel holdoff first send secondary");
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel holdoff secondary send completed");

	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "channel holdoff second recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel holdoff switches primary");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 2U, "channel holdoff integration total switch count");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "channel holdoff second recovery send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel holdoff second send primary");
}

static void vTestIntegratedChannelUpRefreshHoldoffFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xTransportEventFrame;
	rsrx_transport_frame_t axFrames[1];
	rsrx_transport_status_t aeStatuses[1];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x7BU, 0x7CU };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel up holdoff integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up holdoff integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up holdoff integration session connect");

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
	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel up holdoff integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel up holdoff integration handshake pump");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff integration failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel up holdoff integration failover switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up holdoff integration active secondary");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff integration first refresh");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel up holdoff integration first refresh decision");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up holdoff integration still secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel up holdoff integration no switch on first refresh");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "channel up holdoff integration first refresh send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up holdoff integration first refresh send secondary");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff integration clear outstanding");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff integration second refresh");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel up holdoff integration second refresh decision");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "channel up holdoff integration second refresh switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up holdoff integration switched primary");
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "channel up holdoff integration query primary");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up holdoff integration query confirms primary");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "channel up holdoff integration second refresh send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up holdoff integration second refresh send primary");
}

static void vTestIntegratedChannelUpFlapResetFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xTransportEventFrame;
	rsrx_transport_frame_t axFrames[1];
	rsrx_transport_status_t aeStatuses[1];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x7DU, 0x7EU };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel up flap integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up flap integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up flap integration session connect");

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
	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel up flap integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel up flap integration handshake pump");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up flap integration failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel up flap integration first switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up flap integration active secondary");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up flap integration first refresh");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel up flap integration first refresh decision");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel up flap integration no switch on first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up flap integration still secondary after first refresh");

	/* cppcheck-suppress redundantAssignment */
	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up flap integration flap down");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel up flap integration no extra switch on flap down");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up flap integration remains secondary after flap down");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up flap integration second refresh");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel up flap integration second refresh decision");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel up flap integration second refresh still held");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up flap integration still secondary after second refresh");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up flap integration third refresh");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel up flap integration third refresh decision");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "channel up flap integration switch after renewed holdoff");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up flap integration switched primary");
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "channel up flap integration query primary");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up flap integration query confirms primary");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "channel up flap integration final send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up flap integration final send primary");
}

static void vTestIntegratedFailoverTransientRecoveryFlow(void)
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
	rsrx_transport_frame_t xChannelDownFrame;
	rsrx_transport_frame_t xSendFailedFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondaryDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x01U, 0x02U, 0x03U, 0x04U, 0U, 0U, 0U, 0U };
	static const uint8_t auSecondaryPayload[2] = { 0xD3U, 0xD4U };
	size_t xHandshakeLength;
	size_t xSecondaryDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "failover transient integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover transient integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover transient integration session connect");

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
		auSecondaryPayload,
		sizeof(auSecondaryPayload),
		auSecondaryDataFrame,
		sizeof(auSecondaryDataFrame),
		&xSecondaryDataLength);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "failover transient integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover transient integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover transient integration established");

	xTransport.uPrimaryAvailable = 0U;
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover transient integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover transient integration active secondary");

	xSendFailedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xSendFailedFrame.puPayload = (const uint8_t *)0;
	xSendFailedFrame.xPayloadLength = 0U;
	xSendFailedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover transient integration secondary send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover transient integration send on secondary");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover transient integration send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "failover transient integration send budget one");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover transient integration receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "failover transient integration receive budget one");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover transient integration state retained after faults");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auSecondaryDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xSecondaryDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover transient integration inbound recovery");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover transient integration recovered established");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "failover transient integration send budget cleared");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "failover transient integration receive budget cleared");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "failover transient integration send reset count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "failover transient integration receive reset count");
	vAssertTrue(xApplication.uCallCount == 1U, "failover transient integration application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "failover transient integration no lifecycle callback");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover transient integration final secondary send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover transient integration final send secondary");
}

static void vTestIntegratedFailoverTransientSoakFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t xChannelDownFrame;
	rsrx_transport_frame_t xSendFailedFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondaryDataFrame1[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondaryDataFrame2[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x11U, 0x22U, 0x33U, 0x44U, 0U, 0U, 0U, 0U };
	static const uint8_t auSecondaryPayload1[2] = { 0xC3U, 0xC4U };
	static const uint8_t auSecondaryPayload2[2] = { 0xD5U, 0xD6U };
	size_t xHandshakeLength;
	size_t xSecondaryDataLength1;
	size_t xSecondaryDataLength2;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "failover soak integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover soak integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover soak integration session connect");

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
		auSecondaryPayload1,
		sizeof(auSecondaryPayload1),
		auSecondaryDataFrame1,
		sizeof(auSecondaryDataFrame1),
		&xSecondaryDataLength1);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auSecondaryPayload2,
		sizeof(auSecondaryPayload2),
		auSecondaryDataFrame2,
		sizeof(auSecondaryDataFrame2),
		&xSecondaryDataLength2);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "failover soak integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover soak integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover soak integration established");

	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	xSendFailedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xSendFailedFrame.puPayload = (const uint8_t *)0;
	xSendFailedFrame.xPayloadLength = 0U;
	xSendFailedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	xTransport.uPrimaryAvailable = 0U;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover soak integration first failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "failover soak integration first switch");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover soak integration first active secondary");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover soak integration first secondary send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover soak integration first send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover soak integration first receive error");
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auSecondaryDataFrame1;
	xTransport.axReceiveFrames[0].xPayloadLength = xSecondaryDataLength1;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover soak integration first recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "failover soak integration first send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "failover soak integration first receive reset");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "failover soak integration recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "failover soak integration recovery primary");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 2U, "failover soak integration primary recovery switch count");

	/* cppcheck-suppress redundantAssignment */
	xTransport.uPrimaryAvailable = 0U;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover soak integration second failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 3U, "failover soak integration second failover switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover soak integration second active secondary");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover soak integration second secondary send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover soak integration second send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover soak integration second receive error");
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auSecondaryDataFrame2;
	xTransport.axReceiveFrames[0].xPayloadLength = xSecondaryDataLength2;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover soak integration second recovery");

	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover soak integration final established");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 2U, "failover soak integration cumulative send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 2U, "failover soak integration cumulative receive reset");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "failover soak integration final send budget clear");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "failover soak integration final receive budget clear");
	vAssertTrue(xApplication.uCallCount == 2U, "failover soak integration application callbacks");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "failover soak integration no lifecycle callback");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover soak integration final send secondary");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover soak integration final route secondary");
}

static void vTestIntegratedSendFailureFailoverBudgetResetFlow(void)
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
	rsrx_transport_frame_t xChannelDownFrame;
	rsrx_transport_frame_t xSendFailedFrame;
	rsrx_transport_channel_id_t eOutstandingChannelId;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x31U, 0x32U, 0x33U, 0x34U, 0U, 0U, 0U, 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "send failure failover reset integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failure failover reset integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failure failover reset integration session connect");

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
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "send failure failover reset integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "send failure failover reset integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failure failover reset integration established");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "send failure failover reset integration primary send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "send failure failover reset integration primary route");

	xSendFailedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendFailedFrame.puPayload = (const uint8_t *)0;
	xSendFailedFrame.xPayloadLength = 0U;
	xSendFailedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failure failover reset integration primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "send failure failover reset integration primary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "send failure failover reset integration budget primary");

	xTransport.uPrimaryAvailable = 0U;
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failure failover reset integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "send failure failover reset integration active secondary");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "send failure failover reset integration budget cleared on switch");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_CHANNEL_DOWN, "send failure failover reset integration budget reset update");
	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "send failure failover reset integration secondary send");
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xSession.xTransportAdapter) == 1U, "send failure failover reset integration outstanding set");
	eOutstandingChannelId = rsrx_transport_adapter_get_outstanding_send_channel(&xSession.xTransportAdapter);
	vAssertTrue(eOutstandingChannelId == rsrx_channel_manager_get_active_channel(&xSession.xChannelManager), "send failure failover reset integration outstanding matches active channel");

	xSendFailedFrame.eChannelId = eOutstandingChannelId;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failure failover reset integration secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "send failure failover reset integration secondary budget restarted");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == eOutstandingChannelId, "send failure failover reset integration budget secondary");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "send failure failover reset integration restarted increment");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failure failover reset integration final established");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "send failure failover reset integration no lifecycle callback");
}

static void vTestIntegratedPreferredRecoverySendBudgetIsolationFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x41U, 0x42U, 0x43U, 0x44U, 0U, 0U, 0U, 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "preferred recovery budget isolation session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery budget isolation session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery budget isolation session connect");

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
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery budget isolation supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery budget isolation handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery budget isolation established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery budget isolation failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "preferred recovery budget isolation active secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "preferred recovery budget isolation secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery budget isolation secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "preferred recovery budget isolation secondary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "preferred recovery budget isolation budget secondary");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "preferred recovery budget isolation recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery budget isolation selected primary");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery budget isolation active primary");
	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "preferred recovery budget isolation primary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery budget isolation primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "preferred recovery budget isolation primary budget restarted");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery budget isolation budget primary");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_AND_INCREMENT_ON_CHANNEL_SWITCH, "preferred recovery budget isolation reset on channel switch");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery budget isolation final established");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "preferred recovery budget isolation no lifecycle callback");
}

static void vTestIntegratedPreferredRecoveryReceiveErrorCarryoverFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "preferred recovery receive carryover integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery receive carryover integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery receive carryover integration session connect");

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
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery receive carryover integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery receive carryover integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery receive carryover integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery receive carryover integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "preferred recovery receive carryover integration active secondary");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery receive carryover integration secondary first error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "preferred recovery receive carryover integration budget one");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "preferred recovery receive carryover integration no reset before recovery");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "preferred recovery receive carryover integration recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery receive carryover integration selected primary");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery receive carryover integration active primary");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "preferred recovery receive carryover integration budget retained");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "preferred recovery receive carryover integration reset count unchanged");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery receive carryover integration primary next error");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "preferred recovery receive carryover integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "preferred recovery receive carryover integration budget reset after escalation");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "preferred recovery receive carryover integration reset count after escalation");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED, "preferred recovery receive carryover integration escalation decision");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "preferred recovery receive carryover integration escalation reason");
	vAssertTrue(xApplication.uCallCount == 0U, "preferred recovery receive carryover integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "preferred recovery receive carryover integration lifecycle callback");
}

static void vTestIntegratedPreferredRecoveryReceiveErrorResetFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x31U, 0x32U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "preferred recovery receive reset integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery receive reset integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery receive reset integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery receive reset integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery receive reset integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery receive reset integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery receive reset integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "preferred recovery receive reset integration active secondary");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery receive reset integration secondary first error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "preferred recovery receive reset integration budget one");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "preferred recovery receive reset integration recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery receive reset integration selected primary");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery receive reset integration active primary");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "preferred recovery receive reset integration budget retained before success");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery receive reset integration inbound success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery receive reset integration state retained");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "preferred recovery receive reset integration budget cleared");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "preferred recovery receive reset integration reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED, "preferred recovery receive reset integration accepted decision");
	vAssertTrue(xApplication.uCallCount == 1U, "preferred recovery receive reset integration application callback");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery receive reset integration error after success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery receive reset integration state retained after restart");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "preferred recovery receive reset integration budget restarted");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "preferred recovery receive reset integration reset count retained");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "preferred recovery receive reset integration budgeted again");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "preferred recovery receive reset integration ignored again");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "preferred recovery receive reset integration no lifecycle callback");
}

static void vTestIntegratedPreferredRecoveryMixedTransientResetFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x21U, 0x22U, 0x23U, 0x24U, 0U, 0U, 0U, 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x71U, 0x72U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "preferred recovery mixed transient integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery mixed transient integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "preferred recovery mixed transient integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery mixed transient integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery mixed transient integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery mixed transient integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery mixed transient integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "preferred recovery mixed transient integration active secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "preferred recovery mixed transient integration secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery mixed transient integration secondary send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "preferred recovery mixed transient integration send budget one");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery mixed transient integration secondary receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "preferred recovery mixed transient integration receive budget one");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "preferred recovery mixed transient integration recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery mixed transient integration selected primary");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery mixed transient integration active primary");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "preferred recovery mixed transient integration inbound success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "preferred recovery mixed transient integration state retained");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "preferred recovery mixed transient integration send budget cleared");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "preferred recovery mixed transient integration receive budget cleared");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "preferred recovery mixed transient integration send reset count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "preferred recovery mixed transient integration receive reset count");
	vAssertTrue(xApplication.uCallCount == 1U, "preferred recovery mixed transient integration application callback");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "preferred recovery mixed transient integration primary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery mixed transient integration primary send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "preferred recovery mixed transient integration send budget restarted");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "preferred recovery mixed transient integration primary receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "preferred recovery mixed transient integration receive budget restarted");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "preferred recovery mixed transient integration receive budgeted again");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "preferred recovery mixed transient integration no lifecycle callback");
}

static void vTestIntegratedHoldoffSendBudgetIsolationFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x51U, 0x52U, 0x53U, 0x54U, 0U, 0U, 0U, 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "holdoff budget isolation integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff budget isolation integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff budget isolation integration session connect");

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
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "holdoff budget isolation integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff budget isolation integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff budget isolation integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff budget isolation integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff budget isolation integration active secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "holdoff budget isolation integration secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff budget isolation integration secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff budget isolation integration secondary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff budget isolation integration budget secondary");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff budget isolation integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff budget isolation integration still secondary");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff budget isolation integration budget retained during holdoff");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff budget isolation integration budget still secondary");

	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff budget isolation integration second refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff budget isolation integration switched primary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "holdoff budget isolation integration primary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff budget isolation integration primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff budget isolation integration primary budget restarted");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff budget isolation integration budget primary");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_AND_INCREMENT_ON_CHANNEL_SWITCH, "holdoff budget isolation integration reset on switch");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff budget isolation integration final established");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "holdoff budget isolation integration no lifecycle callback");
}

static void vTestIntegratedHoldoffStaleFeedbackIsolationFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x33U, 0x34U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "holdoff stale feedback integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff stale feedback integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff stale feedback integration session connect");

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
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff stale feedback integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback integration active secondary");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback integration hold secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "holdoff stale feedback integration secondary send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback integration outstanding secondary");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback integration stale primary failure");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED, "holdoff stale feedback integration stale primary decision");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff stale feedback integration stale primary budget unchanged");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_NONE, "holdoff stale feedback integration stale primary budget update");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback integration correlated secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff stale feedback integration correlated secondary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback integration correlated secondary budget channel");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff stale feedback integration state retained");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "holdoff stale feedback integration no lifecycle callback");
}

static void vTestIntegratedHoldoffStaleFeedbackRecoveryOrderingFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x35U, 0x36U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "holdoff stale feedback recovery ordering integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff stale feedback recovery ordering integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff stale feedback recovery ordering integration session connect");

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
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback recovery ordering integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback recovery ordering integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff stale feedback recovery ordering integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback recovery ordering integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback recovery ordering integration active secondary");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback recovery ordering integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback recovery ordering integration hold secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "holdoff stale feedback recovery ordering integration secondary send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback recovery ordering integration outstanding secondary");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback recovery ordering integration stale primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff stale feedback recovery ordering integration stale primary budget unchanged");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback recovery ordering integration second refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff stale feedback recovery ordering integration switched primary");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback recovery ordering integration stale secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff stale feedback recovery ordering integration stale secondary budget unchanged");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_NONE, "holdoff stale feedback recovery ordering integration stale secondary budget update");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "holdoff stale feedback recovery ordering integration primary send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff stale feedback recovery ordering integration outstanding primary");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback recovery ordering integration correlated primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff stale feedback recovery ordering integration primary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff stale feedback recovery ordering integration primary budget channel");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff stale feedback recovery ordering integration state retained");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "holdoff stale feedback recovery ordering integration no lifecycle callback");
}

static void vTestIntegratedHoldoffStaleFeedbackSoakFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame1[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame2[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame3[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x41U, 0x42U };
	static const uint8_t auInboundDataPayload1[2] = { 0x51U, 0x52U };
	static const uint8_t auInboundDataPayload2[2] = { 0x53U, 0x54U };
	static const uint8_t auInboundDataPayload3[2] = { 0x55U, 0x56U };
	size_t xHandshakeLength;
	size_t xDataLength1;
	size_t xDataLength2;
	size_t xDataLength3;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "holdoff stale feedback soak integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff stale feedback soak integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff stale feedback soak integration session connect");

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
		auInboundDataPayload1,
		sizeof(auInboundDataPayload1),
		auDataFrame1,
		sizeof(auDataFrame1),
		&xDataLength1);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auInboundDataPayload2,
		sizeof(auInboundDataPayload2),
		auDataFrame2,
		sizeof(auDataFrame2),
		&xDataLength2);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auInboundDataPayload3,
		sizeof(auInboundDataPayload3),
		auDataFrame3,
		sizeof(auDataFrame3),
		&xDataLength3);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback soak integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback soak integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff stale feedback soak integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration first failover");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration first hold");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback soak integration held secondary one");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "holdoff stale feedback soak integration first secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration first stale primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff stale feedback soak integration first stale primary unchanged");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration first correlated secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff stale feedback soak integration first secondary budget one");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame1;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength1;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback soak integration first secondary success");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "holdoff stale feedback soak integration first reset count");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration first recovery");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff stale feedback soak integration primary after first cycle");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration first stale secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff stale feedback soak integration first stale secondary unchanged");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "holdoff stale feedback soak integration primary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration correlated primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff stale feedback soak integration primary budget one");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame2;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength2;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback soak integration primary success");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 2U, "holdoff stale feedback soak integration second reset count");

	// cppcheck-suppress knownConditionTrueFalse
	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "holdoff stale feedback soak integration primary available before second failover");
	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration second failover");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration second hold");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback soak integration held secondary two");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "holdoff stale feedback soak integration second secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration second stale primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff stale feedback soak integration second stale primary unchanged");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration second correlated secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff stale feedback soak integration second secondary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff stale feedback soak integration second budget channel");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame3;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength3;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff stale feedback soak integration second secondary success");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 3U, "holdoff stale feedback soak integration third reset count");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff stale feedback soak integration second recovery");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 4U, "holdoff stale feedback soak integration final switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff stale feedback soak integration final primary");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff stale feedback soak integration state retained");
	vAssertTrue(xApplication.uCallCount == 3U, "holdoff stale feedback soak integration application callback count");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "holdoff stale feedback soak integration no lifecycle callback");
}

static void vTestIntegratedHoldoffReceiveErrorCarryoverFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "holdoff receive carryover integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff receive carryover integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff receive carryover integration session connect");

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
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "holdoff receive carryover integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff receive carryover integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff receive carryover integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff receive carryover integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff receive carryover integration active secondary");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff receive carryover integration secondary error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff receive carryover integration secondary budget one");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "holdoff receive carryover integration no reset yet");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff receive carryover integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff receive carryover integration still secondary");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff receive carryover integration budget retained during holdoff");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff receive carryover integration second refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff receive carryover integration switched primary");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff receive carryover integration budget retained after switch");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff receive carryover integration primary next error");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "holdoff receive carryover integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "holdoff receive carryover integration budget reset after escalation");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "holdoff receive carryover integration reset count after escalation");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED, "holdoff receive carryover integration escalation decision");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "holdoff receive carryover integration escalation reason");
	vAssertTrue(xApplication.uCallCount == 0U, "holdoff receive carryover integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "holdoff receive carryover integration lifecycle callback");
}

static void vTestIntegratedHoldoffMixedTransientResetFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x81U, 0x82U, 0x83U, 0x84U, 0U, 0U, 0U, 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x91U, 0x92U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "holdoff mixed transient integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff mixed transient integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff mixed transient integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "holdoff mixed transient integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff mixed transient integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff mixed transient integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff mixed transient integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff mixed transient integration active secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "holdoff mixed transient integration secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff mixed transient integration secondary send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff mixed transient integration send budget one");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff mixed transient integration secondary receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff mixed transient integration receive budget one");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff mixed transient integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff mixed transient integration still secondary");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff mixed transient integration send budget retained during holdoff");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff mixed transient integration receive budget retained during holdoff");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff mixed transient integration second refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff mixed transient integration switched primary");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff mixed transient integration inbound success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff mixed transient integration state retained");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff mixed transient integration send budget cleared");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "holdoff mixed transient integration receive budget cleared");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "holdoff mixed transient integration send reset count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "holdoff mixed transient integration receive reset count");
	vAssertTrue(xApplication.uCallCount == 1U, "holdoff mixed transient integration application callback");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "holdoff mixed transient integration primary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff mixed transient integration primary send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff mixed transient integration send budget restarted");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff mixed transient integration primary receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff mixed transient integration receive budget restarted");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "holdoff mixed transient integration no lifecycle callback");
}

static void vTestIntegratedHoldoffFlapTransientAsymmetryFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0xA1U, 0xA2U, 0xA3U, 0xA4U, 0U, 0U, 0U, 0U };
	static const uint8_t auInboundDataPayload[2] = { 0xB1U, 0xB2U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "holdoff flap transient integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff flap transient integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "holdoff flap transient integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "holdoff flap transient integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff flap transient integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff flap transient integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff flap transient integration active secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "holdoff flap transient integration secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration secondary send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff flap transient integration send budget one");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration secondary receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff flap transient integration receive budget one");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff flap transient integration still secondary after first refresh");
	// cppcheck-suppress knownConditionTrueFalse
	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "holdoff flap transient integration primary available before flap");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff flap transient integration send budget retained after first refresh");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff flap transient integration receive budget retained after first refresh");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration flap down");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff flap transient integration still secondary after flap");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff flap transient integration send budget reset on flap");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "holdoff flap transient integration send reset count after flap");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff flap transient integration receive budget retained after flap");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration second refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff flap transient integration still secondary after second refresh");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration third refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff flap transient integration switched primary");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "holdoff flap transient integration inbound success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "holdoff flap transient integration state retained");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "holdoff flap transient integration send budget remains clear");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "holdoff flap transient integration receive budget cleared");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "holdoff flap transient integration send reset count retained");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "holdoff flap transient integration receive reset count");
	vAssertTrue(xApplication.uCallCount == 1U, "holdoff flap transient integration application callback");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "holdoff flap transient integration primary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration primary send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "holdoff flap transient integration send budget restarted");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "holdoff flap transient integration primary receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "holdoff flap transient integration receive budget restarted");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "holdoff flap transient integration no lifecycle callback");
}

static void vTestIntegratedRedundancyFlapSoakFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xChannelDownFrame;
	rsrx_transport_frame_t axFrames[1];
	rsrx_transport_status_t aeStatuses[1];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x7BU, 0x7CU };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "redundancy flap integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "redundancy flap integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "redundancy flap integration session connect");

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
	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "redundancy flap integration established");

	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap integration first failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "redundancy flap integration first switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap integration first active secondary");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap integration first recovery query one");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap integration first recovery held");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 1U, "redundancy flap integration first total switch count");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap integration first held send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap integration first held send secondary");
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap integration first held send completed");

	/* cppcheck-suppress redundantAssignment */
	xTransport.uPrimaryAvailable = 0U;
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap integration flap reset event");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "redundancy flap integration no extra switch on flap");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap integration second recovery query one");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap integration second recovery still held");
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap integration second recovery query two");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap integration second recovery switches primary");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 2U, "redundancy flap integration second recovery total switch count");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap integration second recovery send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap integration second recovery send primary");
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap integration second recovery send completed");

	/* cppcheck-suppress redundantAssignment */
	xTransport.uPrimaryAvailable = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap integration third failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 3U, "redundancy flap integration third switch count");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 1U, "redundancy flap integration third switch occurred");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap integration third active secondary");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap integration final recovery query one");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap integration final recovery held");
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap integration final recovery query two");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap integration final recovery primary");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 4U, "redundancy flap integration final total switch count");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap integration final send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap integration final send primary");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "redundancy flap integration state retained");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "redundancy flap integration no lifecycle callback");
}

static void vTestIntegratedRedundancyFlapTransientSoakFlow(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_transport_channel_state_t xChannelState;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	rsrx_codec_port_t xCodec = *rsrx_codec_get_default_port();
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame1[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame2[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xTransportEventFrame;
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x7DU, 0x7EU };
	static const uint8_t auInboundDataPayload1[2] = { 0x7FU, 0x80U };
	static const uint8_t auInboundDataPayload2[2] = { 0x81U, 0x82U };
	size_t xHandshakeLength;
	size_t xDataLength1;
	size_t xDataLength2;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "redundancy flap transient soak integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "redundancy flap transient soak integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "redundancy flap transient soak integration session connect");

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
		auInboundDataPayload1,
		sizeof(auInboundDataPayload1),
		auDataFrame1,
		sizeof(auDataFrame1),
		&xDataLength1);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auInboundDataPayload2,
		sizeof(auInboundDataPayload2),
		auDataFrame2,
		sizeof(auDataFrame2),
		&xDataLength2);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient soak integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient soak integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "redundancy flap transient soak integration established");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient soak integration first failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "redundancy flap transient soak integration first switch count");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap transient soak integration secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient soak integration secondary send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient soak integration secondary receive error");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame1;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength1;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient soak integration secondary recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "redundancy flap transient soak integration first send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "redundancy flap transient soak integration first receive reset");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap transient soak integration recovery query one");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap transient soak integration recovery held");
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap transient soak integration recovery query two");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap transient soak integration recovery primary");
	// cppcheck-suppress knownConditionTrueFalse
	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "redundancy flap transient soak integration primary available before second failover");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 2U, "redundancy flap transient soak integration second switch count");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient soak integration second failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 3U, "redundancy flap transient soak integration third switch count");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap transient soak integration second secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient soak integration second secondary send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient soak integration second secondary receive error");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame2;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength2;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient soak integration second secondary recovery");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "redundancy flap transient soak integration state retained after second secondary recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 2U, "redundancy flap transient soak integration second send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 2U, "redundancy flap transient soak integration second receive reset");

	xTransport.uPrimaryAvailable = 1U;
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap transient soak integration final recovery query one");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap transient soak integration final recovery held");
	vAssertTrue(rsrx_transport_adapter_query_channel(&xSession.xTransportAdapter, &xChannelState) == RSRX_TRANSPORT_STATUS_OK, "redundancy flap transient soak integration final recovery query two");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap transient soak integration final recovery primary");
	// cppcheck-suppress knownConditionTrueFalse
	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "redundancy flap transient soak integration primary available before final completion");
	vAssertTrue(xSession.xChannelManager.uTotalSwitchCount == 4U, "redundancy flap transient soak integration final switch count");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "redundancy flap transient soak integration state retained");
	vAssertTrue(xApplication.uCallCount == 2U, "redundancy flap transient soak integration application callback count");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "redundancy flap transient soak integration no lifecycle callback");
}

static void vTestIntegratedChannelUpHoldoffTransientSoakFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame1[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame2[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xTransportEventFrame;
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload1[2] = { 0x81U, 0x82U };
	static const uint8_t auInboundDataPayload2[2] = { 0x83U, 0x84U };
	size_t xHandshakeLength;
	size_t xDataLength1;
	size_t xDataLength2;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel up holdoff transient soak integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up holdoff transient soak integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up holdoff transient soak integration session connect");

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
		auInboundDataPayload1,
		sizeof(auInboundDataPayload1),
		auDataFrame1,
		sizeof(auDataFrame1),
		&xDataLength1);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auInboundDataPayload2,
		sizeof(auInboundDataPayload2),
		auDataFrame2,
		sizeof(auDataFrame2),
		&xDataLength2);
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel up holdoff transient soak integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel up holdoff transient soak integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel up holdoff transient soak integration established");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration first failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel up holdoff transient soak integration first switch count");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration first refresh");
	// cppcheck-suppress knownConditionTrueFalse
	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "channel up holdoff transient soak integration primary available during holdoff");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up holdoff transient soak integration held secondary");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "channel up holdoff transient soak integration first secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration first secondary send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration first secondary receive error");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame1;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength1;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel up holdoff transient soak integration first secondary recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "channel up holdoff transient soak integration first send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "channel up holdoff transient soak integration first receive reset");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration second refresh");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "channel up holdoff transient soak integration second switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up holdoff transient soak integration switched primary");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration second failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 3U, "channel up holdoff transient soak integration third switch count");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration third refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up holdoff transient soak integration held secondary again");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "channel up holdoff transient soak integration second secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration second secondary send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration second secondary receive error");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame2;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength2;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "channel up holdoff transient soak integration second secondary recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 2U, "channel up holdoff transient soak integration second send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 2U, "channel up holdoff transient soak integration second receive reset");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up holdoff transient soak integration fourth refresh");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 4U, "channel up holdoff transient soak integration final switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up holdoff transient soak integration final primary");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel up holdoff transient soak integration state retained");
	vAssertTrue(xApplication.uCallCount == 2U, "channel up holdoff transient soak integration application callback count");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "channel up holdoff transient soak integration no lifecycle callback");
}

static void vTestIntegratedRedundancyFlapTransientLongRunFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame1[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame2[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame3[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xTransportEventFrame;
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auOutboundPayload[2] = { 0x71U, 0x72U };
	static const uint8_t auInboundDataPayload1[2] = { 0x73U, 0x74U };
	static const uint8_t auInboundDataPayload2[2] = { 0x75U, 0x76U };
	static const uint8_t auInboundDataPayload3[2] = { 0x77U, 0x78U };
	size_t xHandshakeLength;
	size_t xDataLength1;
	size_t xDataLength2;
	size_t xDataLength3;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyHoldoffConfig(&xConfig, 2U);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "redundancy flap transient long run integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "redundancy flap transient long run integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "redundancy flap transient long run integration session connect");

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
		auInboundDataPayload1,
		sizeof(auInboundDataPayload1),
		auDataFrame1,
		sizeof(auDataFrame1),
		&xDataLength1);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auInboundDataPayload2,
		sizeof(auInboundDataPayload2),
		auDataFrame2,
		sizeof(auDataFrame2),
		&xDataLength2);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		4U,
		1U,
		auInboundDataPayload3,
		sizeof(auInboundDataPayload3),
		auDataFrame3,
		sizeof(auDataFrame3),
		&xDataLength3);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient long run integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient long run integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "redundancy flap transient long run integration established");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration first failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "redundancy flap transient long run integration first switch count");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration first refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap transient long run integration held secondary one");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap transient long run integration first secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration first secondary send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration first secondary receive error");
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame1;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength1;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient long run integration first secondary recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "redundancy flap transient long run integration first send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "redundancy flap transient long run integration first receive reset");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration second refresh");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "redundancy flap transient long run integration second switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap transient long run integration primary after first cycle");
	// cppcheck-suppress knownConditionTrueFalse
	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "redundancy flap transient long run integration primary available before second failover");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration second failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 3U, "redundancy flap transient long run integration third switch count");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration third refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap transient long run integration held secondary two");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap transient long run integration second secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration second secondary send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration second secondary receive error");
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame2;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength2;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient long run integration second secondary recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 2U, "redundancy flap transient long run integration second send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 2U, "redundancy flap transient long run integration second receive reset");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration fourth refresh");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 4U, "redundancy flap transient long run integration fourth switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap transient long run integration primary after second cycle");
	// cppcheck-suppress knownConditionTrueFalse
	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "redundancy flap transient long run integration primary available before third failover");

	xTransport.uPrimaryAvailable = 0U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration third failover");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 5U, "redundancy flap transient long run integration fifth switch count");

	xTransport.uPrimaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration fifth refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "redundancy flap transient long run integration held secondary three");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auOutboundPayload, sizeof(auOutboundPayload)) == RSRX_STATUS_OK, "redundancy flap transient long run integration third secondary send");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration third secondary send failure");
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration third secondary receive error");
	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame3;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength3;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "redundancy flap transient long run integration third secondary recovery");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 3U, "redundancy flap transient long run integration third send reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 3U, "redundancy flap transient long run integration third receive reset");

	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "redundancy flap transient long run integration sixth refresh");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 6U, "redundancy flap transient long run integration final switch count");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "redundancy flap transient long run integration final primary");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "redundancy flap transient long run integration state retained");
	vAssertTrue(xApplication.uCallCount == 3U, "redundancy flap transient long run integration application callback count");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "redundancy flap transient long run integration no lifecycle callback");
}

static void vTestIntegratedDecodeFailureFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auCorruptFrame[3] = { 0x99U, 0x88U, 0x77U };
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "decode failure integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "decode failure integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "decode failure integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "decode failure integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "decode failure integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "decode failure integration established");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auCorruptFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = sizeof(auCorruptFrame);
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_DECODE_FAILED, "decode failure integration poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "decode failure integration state retained");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_DECODE_FAILED, "decode failure integration decision");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 1U, "decode failure integration processed count retained");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 1U, "decode failure integration prior pump count retained");
	vAssertTrue(xApplication.uCallCount == 0U, "decode failure integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "decode failure integration no lifecycle callback");
}

static void vTestIntegratedSendFailureBudgetFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xSendFailedFrame;
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "send failure integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failure integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failure integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "send failure integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "send failure integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failure integration established");

	xSendFailedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendFailedFrame.puPayload = (const uint8_t *)0;
	xSendFailedFrame.xPayloadLength = 0U;
	xSendFailedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "send failure integration priming send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failure integration first failure");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failure integration state retained after first");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "send failure integration budget count");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "send failure integration budget update");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED, "send failure integration budget decision");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "send failure integration no lifecycle on first");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "send failure integration second failure");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "send failure integration safe disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "send failure integration reason");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_REJECTED, "send failure integration escalation decision");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "send failure integration budget reset");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_ESCALATION, "send failure integration escalation budget update");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "send failure integration reset count");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "send failure integration lifecycle on escalation");
}

static void vTestIntegratedSendFailureBudgetResetFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xSendFailedFrame;
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x11U, 0x22U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "send failure reset integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failure reset integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failure reset integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "send failure reset integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "send failure reset integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failure reset integration established");

	xSendFailedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendFailedFrame.puPayload = (const uint8_t *)0;
	xSendFailedFrame.xPayloadLength = 0U;
	xSendFailedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "send failure reset integration priming send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failure reset integration first failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "send failure reset integration budget count one");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "send failure reset integration incremented");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "send failure reset integration inbound success");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "send failure reset integration budget cleared");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_INBOUND_FRAME, "send failure reset integration inbound reset");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "send failure reset integration reset count");
	vAssertTrue(xApplication.uCallCount == 1U, "send failure reset integration application callback");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "send failure reset integration second priming send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failure reset integration failure after success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failure reset integration state retained");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "send failure reset integration budget restarted");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "send failure reset integration restarted increment");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED, "send failure reset integration budgeted again");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "send failure reset integration no lifecycle callback");
}

static void vTestIntegratedReceiveErrorBudgetFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "receive error integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "receive error integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "receive error integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "receive error integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "receive error integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "receive error integration established");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "receive error integration first error");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "receive error integration state retained after first");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "receive error integration budget count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "receive error integration no reset yet");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "receive error integration budget decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "receive error integration budget decision class");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "receive error integration no lifecycle on first");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "receive error integration second error");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "receive error integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "receive error integration budget reset");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "receive error integration reset count");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "receive error integration escalated event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "receive error integration session rejected");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED, "receive error integration escalation decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_ERROR, "receive error integration escalation class");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "receive error integration escalation reason");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "receive error integration lifecycle on escalation");
}

static void vTestIntegratedReceiveErrorBudgetResetFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x55U, 0x66U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "receive error reset integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "receive error reset integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "receive error reset integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "receive error reset integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "receive error reset integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "receive error reset integration established");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "receive error reset integration first error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "receive error reset integration budget one");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "receive error reset integration no reset yet");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "receive error reset integration inbound success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "receive error reset integration state retained");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "receive error reset integration budget cleared");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "receive error reset integration reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED, "receive error reset integration accepted decision");
	vAssertTrue(xApplication.uCallCount == 1U, "receive error reset integration application callback");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "receive error reset integration error after success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "receive error reset integration state retained after restart");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "receive error reset integration budget restarted");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "receive error reset integration reset count retained");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "receive error reset integration budgeted again");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "receive error reset integration ignored again");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "receive error reset integration no lifecycle callback");
}

static void vTestIntegratedReceiveErrorFailoverCarryoverFlow(void)
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
	rsrx_transport_frame_t xChannelDownFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "receive error failover carryover integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "receive error failover carryover integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "receive error failover carryover integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "receive error failover carryover integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "receive error failover carryover integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "receive error failover carryover integration established");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "receive error failover carryover integration primary first error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "receive error failover carryover integration primary budget one");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "receive error failover carryover integration no reset before failover");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xChannelDownFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xChannelDownFrame.puPayload = (const uint8_t *)0;
	xChannelDownFrame.xPayloadLength = 0U;
	xChannelDownFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xChannelDownFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "receive error failover carryover integration failover event");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "receive error failover carryover integration active secondary");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "receive error failover carryover integration budget retained on failover");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "receive error failover carryover integration reset count unchanged");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED, "receive error failover carryover integration failover decision");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "receive error failover carryover integration secondary next error");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "receive error failover carryover integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "receive error failover carryover integration budget reset after escalation");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "receive error failover carryover integration reset count after escalation");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "receive error failover carryover integration protocol error event");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "receive error failover carryover integration reason");
	vAssertTrue(xApplication.uCallCount == 0U, "receive error failover carryover integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "receive error failover carryover integration lifecycle callback");
}

static void vTestIntegratedMixedTransientBudgetResetFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t xSendFailedFrame;
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x61U, 0x62U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "mixed transient integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "mixed transient integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "mixed transient integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "mixed transient integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "mixed transient integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "mixed transient integration established");

	xSendFailedFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendFailedFrame.puPayload = (const uint8_t *)0;
	xSendFailedFrame.xPayloadLength = 0U;
	xSendFailedFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "mixed transient integration priming send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "mixed transient integration send failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "mixed transient integration send budget one");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "mixed transient integration receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "mixed transient integration receive budget one");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "mixed transient integration state retained after faults");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auDataFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xDataLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "mixed transient integration inbound recovery");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "mixed transient integration state retained after recovery");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "mixed transient integration send budget cleared");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "mixed transient integration receive budget cleared");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "mixed transient integration send reset count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "mixed transient integration receive reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED, "mixed transient integration accepted decision");
	vAssertTrue(xApplication.uCallCount == 1U, "mixed transient integration application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 0U, "mixed transient integration no lifecycle callback");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "mixed transient integration second priming send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xSendFailedFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "mixed transient integration send failure after recovery");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "mixed transient integration send budget restarted");

	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.uReceiveScriptIndex = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "mixed transient integration receive error after recovery");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "mixed transient integration receive budget restarted");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "mixed transient integration final established");
}

static void vTestIntegratedDuplicateInboundProtocolErrorFlow(void)
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
	uint8_t auDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x21U, 0x43U };
	size_t xHandshakeLength;
	size_t xDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "duplicate inbound integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "duplicate inbound integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "duplicate inbound integration session connect");

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
		auDataFrame,
		sizeof(auDataFrame),
		&xDataLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auDataFrame;
	axFrames[1].xPayloadLength = xDataLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = auDataFrame;
	axFrames[2].xPayloadLength = xDataLength;
	axFrames[2].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 3U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "duplicate inbound integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "duplicate inbound integration pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "duplicate inbound integration safe disconnect");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 3U, "duplicate inbound integration processed count");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "duplicate inbound integration effective protocol error");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "duplicate inbound integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "duplicate inbound integration reason");
	vAssertTrue(xApplication.uCallCount == 1U, "duplicate inbound integration single application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "duplicate inbound integration lifecycle callback");
}

static void vTestIntegratedInitialZeroSequenceProtocolErrorFlow(void)
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
	uint8_t auInvalidHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	size_t xInvalidHandshakeLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "zero sequence integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "zero sequence integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "zero sequence integration session connect");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_CONNECT_RESPONSE,
		RSRX_REASON_HANDSHAKE_COMPLETED,
		0U,
		0U,
		(const uint8_t *)0,
		0U,
		auInvalidHandshakeFrame,
		sizeof(auInvalidHandshakeFrame),
		&xInvalidHandshakeLength);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auInvalidHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xInvalidHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "zero sequence integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "zero sequence integration poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "zero sequence integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "zero sequence integration effective protocol error");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "zero sequence integration rejected status");
	vAssertTrue(xApplication.uCallCount == 0U, "zero sequence integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "zero sequence integration lifecycle callback");
}

static void vTestIntegratedInvalidConfirmationProtocolErrorFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auInvalidConfirmationFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x10U, 0x20U, 0x30U, 0x40U, 0U, 0U, 0U, 0U };
	static const uint8_t auInboundDataPayload[2] = { 0x71U, 0x72U };
	size_t xHandshakeLength;
	size_t xInvalidConfirmationLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "invalid confirmation integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "invalid confirmation integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "invalid confirmation integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "invalid confirmation integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "invalid confirmation integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "invalid confirmation integration established");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "invalid confirmation integration outbound send");
	vAssertTrue(xTransport.uSendCount == 2U, "invalid confirmation integration outbound high watermark");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		3U,
		auInboundDataPayload,
		sizeof(auInboundDataPayload),
		auInvalidConfirmationFrame,
		sizeof(auInvalidConfirmationFrame),
		&xInvalidConfirmationLength);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auInvalidConfirmationFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xInvalidConfirmationLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "invalid confirmation integration poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "invalid confirmation integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "invalid confirmation integration effective protocol error");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "invalid confirmation integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "invalid confirmation integration reason");
	vAssertTrue(xApplication.uCallCount == 0U, "invalid confirmation integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "invalid confirmation integration lifecycle callback");
}

static void vTestIntegratedFailoverInvalidConfirmationProtocolErrorFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auInvalidConfirmationFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t axFrames[1];
	rsrx_transport_status_t aeStatuses[1];
	static const uint8_t auFramePayload[8] = { 0x31U, 0x32U, 0x33U, 0x34U, 0U, 0U, 0U, 0U };
	static const uint8_t auInboundPayload[2] = { 0xC1U, 0xC2U };
	size_t xHandshakeLength;
	size_t xInvalidConfirmationLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "failover invalid confirmation integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover invalid confirmation integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover invalid confirmation integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "failover invalid confirmation integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover invalid confirmation integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover invalid confirmation integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover invalid confirmation integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover invalid confirmation integration active secondary");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover invalid confirmation integration outbound send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover invalid confirmation integration outbound on secondary");
	vAssertTrue(xTransport.uSendCount == 2U, "failover invalid confirmation integration outbound high watermark");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		4U,
		auInboundPayload,
		sizeof(auInboundPayload),
		auInvalidConfirmationFrame,
		sizeof(auInvalidConfirmationFrame),
		&xInvalidConfirmationLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auInvalidConfirmationFrame;
	axFrames[0].xPayloadLength = xInvalidConfirmationLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover invalid confirmation integration poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "failover invalid confirmation integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "failover invalid confirmation integration effective protocol error");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "failover invalid confirmation integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "failover invalid confirmation integration reason");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "failover invalid confirmation integration switch count");
	vAssertTrue(xApplication.uCallCount == 0U, "failover invalid confirmation integration no application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "failover invalid confirmation integration lifecycle callback");
}

static void vTestIntegratedRegressingConfirmationProtocolErrorFlow(void)
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
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auValidConfirmationFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRegressingConfirmationFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0x50U, 0x60U, 0x70U, 0x80U, 0U, 0U, 0U, 0U };
	static const uint8_t auValidPayload[2] = { 0x81U, 0x82U };
	static const uint8_t auRegressingPayload[2] = { 0x91U, 0x92U };
	size_t xHandshakeLength;
	size_t xValidConfirmationLength;
	size_t xRegressingConfirmationLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "regressing confirmation integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "regressing confirmation integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "regressing confirmation integration session connect");

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

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auHandshakeFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xHandshakeLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptCount = 1U;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "regressing confirmation integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "regressing confirmation integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "regressing confirmation integration established");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "regressing confirmation integration outbound send one");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(
		&xSupervisor,
		&(rsrx_transport_frame_t){ RSRX_TRANSPORT_CHANNEL_PRIMARY, (const uint8_t *)0, 0U, RSRX_TRANSPORT_EVENT_SEND_COMPLETED },
		&pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "regressing confirmation integration clear first outstanding");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "regressing confirmation integration outbound send two");
	vAssertTrue(xTransport.uSendCount == 3U, "regressing confirmation integration outbound count");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		2U,
		auValidPayload,
		sizeof(auValidPayload),
		auValidConfirmationFrame,
		sizeof(auValidConfirmationFrame),
		&xValidConfirmationLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auRegressingPayload,
		sizeof(auRegressingPayload),
		auRegressingConfirmationFrame,
		sizeof(auRegressingConfirmationFrame),
		&xRegressingConfirmationLength);

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auValidConfirmationFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xValidConfirmationLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "regressing confirmation integration valid confirmation poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "regressing confirmation integration state retained after valid");
	vAssertTrue(xApplication.uCallCount == 1U, "regressing confirmation integration valid application callback");

	xTransport.axReceiveFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransport.axReceiveFrames[0].puPayload = auRegressingConfirmationFrame;
	xTransport.axReceiveFrames[0].xPayloadLength = xRegressingConfirmationLength;
	xTransport.axReceiveFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xTransport.aeReceiveStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uReceiveScriptIndex = 0U;

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "regressing confirmation integration regressing poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "regressing confirmation integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "regressing confirmation integration effective protocol error");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "regressing confirmation integration rejected status");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "regressing confirmation integration reason");
	vAssertTrue(xApplication.uCallCount == 1U, "regressing confirmation integration no second application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "regressing confirmation integration lifecycle callback");
}

static void vTestIntegratedFailoverRegressingConfirmationProtocolErrorFlow(void)
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
	rsrx_transport_frame_t xTransportEventFrame;
	uint8_t auHandshakeFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auValidConfirmationFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auRegressingConfirmationFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	static const uint8_t auFramePayload[8] = { 0x41U, 0x42U, 0x43U, 0x44U, 0U, 0U, 0U, 0U };
	static const uint8_t auValidPayload[2] = { 0xD1U, 0xD2U };
	static const uint8_t auRegressingPayload[2] = { 0xD3U, 0xD4U };
	size_t xHandshakeLength;
	size_t xValidConfirmationLength;
	size_t xRegressingConfirmationLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
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
	vSetActiveStandbyConfig(&xConfig);

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "failover regressing confirmation integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover regressing confirmation integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "failover regressing confirmation integration session connect");

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

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "failover regressing confirmation integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover regressing confirmation integration handshake pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover regressing confirmation integration established");

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xTransportEventFrame.puPayload = (const uint8_t *)0;
	xTransportEventFrame.xPayloadLength = 0U;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover regressing confirmation integration failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover regressing confirmation integration active secondary");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover regressing confirmation integration outbound send one");
	xTransportEventFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xTransportEventFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xTransportEventFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "failover regressing confirmation integration clear first outstanding");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auFramePayload, sizeof(auFramePayload)) == RSRX_STATUS_OK, "failover regressing confirmation integration outbound send two");
	vAssertTrue(xTransport.uSendCount == 3U, "failover regressing confirmation integration outbound count");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover regressing confirmation integration outbound on secondary");

	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		2U,
		auValidPayload,
		sizeof(auValidPayload),
		auValidConfirmationFrame,
		sizeof(auValidConfirmationFrame),
		&xValidConfirmationLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auRegressingPayload,
		sizeof(auRegressingPayload),
		auRegressingConfirmationFrame,
		sizeof(auRegressingConfirmationFrame),
		&xRegressingConfirmationLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auValidConfirmationFrame;
	axFrames[0].xPayloadLength = xValidConfirmationLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover regressing confirmation integration valid confirmation poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "failover regressing confirmation integration state retained after valid");
	vAssertTrue(xApplication.uCallCount == 1U, "failover regressing confirmation integration valid application callback");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axFrames[0].puPayload = auRegressingConfirmationFrame;
	axFrames[0].xPayloadLength = xRegressingConfirmationLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "failover regressing confirmation integration regressing poll");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "failover regressing confirmation integration safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "failover regressing confirmation integration effective protocol error");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "failover regressing confirmation integration rejected status");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "failover regressing confirmation integration switch count");
	vAssertTrue(xApplication.uCallCount == 1U, "failover regressing confirmation integration no second application callback");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "failover regressing confirmation integration lifecycle callback");
}

static void vTestIntegratedPumpReceiveStabilityFlow(void)
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
	uint8_t auFirstDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint8_t auSecondDataFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auFirstDataPayload[2] = { 0x31U, 0x32U };
	static const uint8_t auSecondDataPayload[2] = { 0x41U, 0x42U };
	size_t xHandshakeLength;
	size_t xFirstDataLength;
	size_t xSecondDataLength;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump stability integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump stability integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump stability integration session connect");

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
		auFirstDataPayload,
		sizeof(auFirstDataPayload),
		auFirstDataFrame,
		sizeof(auFirstDataFrame),
		&xFirstDataLength);
	vEncodeFrame(
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		1U,
		auSecondDataPayload,
		sizeof(auSecondDataPayload),
		auSecondDataFrame,
		sizeof(auSecondDataFrame),
		&xSecondDataLength);

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auFirstDataFrame;
	axFrames[1].xPayloadLength = xFirstDataLength;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = (const uint8_t *)0;
	axFrames[2].xPayloadLength = 0U;
	axFrames[2].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 3U);

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump stability integration supervisor init");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 4U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "pump stability integration first pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "pump stability integration established");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 2U, "pump stability integration first pump local count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 2U, "pump stability integration first pump total count");
	vAssertTrue(xApplication.uCallCount == 1U, "pump stability integration first callback");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = (const uint8_t *)0;
	axFrames[0].xPayloadLength = 0U;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 1U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 2U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_NO_FRAME, "pump stability integration idle pump");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 0U, "pump stability integration idle local count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 2U, "pump stability integration idle total count");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "pump stability integration idle state");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auSecondDataFrame;
	axFrames[0].xPayloadLength = xSecondDataLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = (const uint8_t *)0;
	axFrames[1].xPayloadLength = 0U;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 2U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "pump stability integration second pump");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 1U, "pump stability integration second pump local count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 3U, "pump stability integration second pump total count");
	vAssertTrue(xApplication.uCallCount == 2U, "pump stability integration second callback");
	vAssertTrue(xApplication.xLastIndication.uSequenceNumber == 3U, "pump stability integration last sequence");
}

static void vTestIntegratedBoundedSoakPumpFlow(void)
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
	uint8_t auDataFrames[5][D_RSRX_CODEC_MAX_FRAME_BYTES];
	static const uint8_t auFramePayload[8] = { 0U };
	static const uint8_t auPayload0[2] = { 0x51U, 0x52U };
	static const uint8_t auPayload1[2] = { 0x53U, 0x54U };
	static const uint8_t auPayload2[2] = { 0x55U, 0x56U };
	static const uint8_t auPayload3[2] = { 0x57U, 0x58U };
	static const uint8_t auPayload4[2] = { 0x59U, 0x5AU };
	const uint8_t * apuPayloads[5] = { auPayload0, auPayload1, auPayload2, auPayload3, auPayload4 };
	size_t axDataLengths[5];
	size_t xHandshakeLength;
	uint32_t uIndex;

	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 0U;
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

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "soak integration session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "soak integration session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "soak integration session connect");

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

	for(uIndex = 0U; uIndex < 5U; ++uIndex)
	{
		vEncodeFrame(
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			uIndex + 2U,
			1U,
			apuPayloads[uIndex],
			2U,
			auDataFrames[uIndex],
			sizeof(auDataFrames[uIndex]),
			&axDataLengths[uIndex]);
	}

	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "soak integration supervisor init");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auHandshakeFrame;
	axFrames[0].xPayloadLength = xHandshakeLength;
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auDataFrames[0];
	axFrames[1].xPayloadLength = axDataLengths[0];
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = auDataFrames[1];
	axFrames[2].xPayloadLength = axDataLengths[1];
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
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 5U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "soak integration first pump");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 3U, "soak integration first local processed count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 3U, "soak integration first total processed count");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auDataFrames[2];
	axFrames[0].xPayloadLength = axDataLengths[2];
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auDataFrames[3];
	axFrames[1].xPayloadLength = axDataLengths[3];
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = (const uint8_t *)0;
	axFrames[2].xPayloadLength = 0U;
	axFrames[2].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 3U);
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 4U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "soak integration second pump");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 2U, "soak integration second local processed count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 5U, "soak integration second total processed count");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auDataFrames[4];
	axFrames[0].xPayloadLength = axDataLengths[4];
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = (const uint8_t *)0;
	axFrames[1].xPayloadLength = 0U;
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "soak integration third pump");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "soak integration established");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 1U, "soak integration third local processed count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 6U, "soak integration total processed count");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 2U, "soak integration third iteration count");
	vAssertTrue(xApplication.uCallCount == 5U, "soak integration application callbacks");
	vAssertTrue(xApplication.xLastIndication.uSequenceNumber == 6U, "soak integration last sequence");
}

int main(void)
{
	vTestIntegratedSessionSupervisorFlow();
	vTestIntegratedDeferredQueueTelemetryFlow();
	vTestIntegratedQueueOverflowRejectFlow();
	vTestIntegratedBusyRejectThresholdEscalationFlow();
	vTestIntegratedRetransmissionRecoveryFlow();
	vTestIntegratedUnconfirmedRecoveryProtocolErrorFlow();
	vTestIntegratedRepeatedGapRetransmissionFlow();
	vTestIntegratedRepeatedGapRecoveryFlow();
	vTestIntegratedRetransmissionTimeoutFailSafeFlow();
	vTestIntegratedRetransmissionFailoverRecoveryFlow();
	vTestIntegratedRetransmissionChannelUpHoldoffRecoveryFlow();
	vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapRecoveryFlow();
	vTestIntegratedRetransmissionChannelUpHoldoffTimeoutFlow();
	vTestIntegratedRetransmissionFailoverTimeoutFlow();
	vTestIntegratedRetransmissionFailoverRepeatedGapRecoveryFlow();
	vTestIntegratedRetransmissionFailoverRepeatedGapTimeoutFlow();
	vTestIntegratedStaleRetransmissionProtocolErrorFlow();
	vTestIntegratedRetransmissionFailoverStaleProtocolErrorFlow();
	vTestIntegratedRetransmissionFailoverUnconfirmedRecoveryProtocolErrorFlow();
	vTestIntegratedRetransmissionFailoverRepeatedGapUnconfirmedRecoveryProtocolErrorFlow();
	vTestIntegratedTimeoutFailSafeFlow();
	vTestIntegratedChannelDownFailSafeFlow();
	vTestIntegratedChannelFailoverFlow();
	vTestIntegratedChannelRecoveryHoldoffFlow();
	vTestIntegratedChannelUpRefreshHoldoffFlow();
	vTestIntegratedChannelUpFlapResetFlow();
	vTestIntegratedFailoverTransientRecoveryFlow();
	vTestIntegratedFailoverTransientSoakFlow();
	vTestIntegratedSendFailureFailoverBudgetResetFlow();
	vTestIntegratedPreferredRecoverySendBudgetIsolationFlow();
	vTestIntegratedPreferredRecoveryReceiveErrorCarryoverFlow();
	vTestIntegratedPreferredRecoveryReceiveErrorResetFlow();
	vTestIntegratedPreferredRecoveryMixedTransientResetFlow();
	vTestIntegratedHoldoffSendBudgetIsolationFlow();
	vTestIntegratedHoldoffStaleFeedbackIsolationFlow();
	vTestIntegratedHoldoffStaleFeedbackRecoveryOrderingFlow();
	vTestIntegratedHoldoffStaleFeedbackSoakFlow();
	vTestIntegratedHoldoffReceiveErrorCarryoverFlow();
	vTestIntegratedHoldoffMixedTransientResetFlow();
	vTestIntegratedHoldoffFlapTransientAsymmetryFlow();
	vTestIntegratedRedundancyFlapSoakFlow();
	vTestIntegratedRedundancyFlapTransientSoakFlow();
	vTestIntegratedRedundancyFlapTransientLongRunFlow();
	vTestIntegratedChannelUpHoldoffTransientSoakFlow();
	vTestIntegratedDecodeFailureFlow();
	vTestIntegratedSendFailureBudgetFlow();
	vTestIntegratedSendFailureBudgetResetFlow();
	vTestIntegratedReceiveErrorBudgetFlow();
	vTestIntegratedReceiveErrorBudgetResetFlow();
	vTestIntegratedReceiveErrorFailoverCarryoverFlow();
	vTestIntegratedMixedTransientBudgetResetFlow();
	vTestIntegratedInitialZeroSequenceProtocolErrorFlow();
	vTestIntegratedDuplicateInboundProtocolErrorFlow();
	vTestIntegratedInvalidConfirmationProtocolErrorFlow();
	vTestIntegratedFailoverInvalidConfirmationProtocolErrorFlow();
	vTestIntegratedRegressingConfirmationProtocolErrorFlow();
	vTestIntegratedFailoverRegressingConfirmationProtocolErrorFlow();
	vTestIntegratedPumpReceiveStabilityFlow();
	vTestIntegratedBoundedSoakPumpFlow();

	(void)printf("rsrx_session_supervisor_flow_test: all tests passed\n");
	return EXIT_SUCCESS;
}
