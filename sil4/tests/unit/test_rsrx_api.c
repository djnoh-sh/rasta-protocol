#include <stdio.h>
#include <stdlib.h>

#include "rsrx_channel_manager.h"
#include "rsrx_api.h"
#include "rsrx_codec.h"

typedef struct
{
	rsrx_monotonic_time_ns_t uNowNs;
} test_clock_context_t;

typedef struct
{
	uint32_t uCallCount;
	rsrx_orchestrator_report_t xLastReport;
} test_counter_t;

typedef struct
{
	rsrx_application_data_indication_t xLastIndication;
	uint32_t uCallCount;
} test_application_context_t;

typedef struct
{
	rsrx_transport_send_request_t xLastRequest;
	uint32_t uSendCount;
} test_transport_context_t;

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

static void vAssertTrue(int iCondition, const char * pcMessage)
{
	if(iCondition == 0)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s\n", pcMessage);
		exit(EXIT_FAILURE);
	}
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

static rsrx_transport_status_t eTransportSend(void * pvContext, const rsrx_transport_send_request_t * pxRequest)
{
	test_transport_context_t * pxContext = (test_transport_context_t *)pvContext;
	pxContext->xLastRequest = *pxRequest;
	pxContext->uSendCount++;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t eTransportReceive(void * pvContext, rsrx_transport_frame_t * pxFrame)
{
	(void)pvContext;
	(void)pxFrame;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t eTransportQuery(void * pvContext, rsrx_transport_channel_state_t * pxState)
{
	(void)pvContext;
	if(pxState != (rsrx_transport_channel_state_t *)0)
	{
		pxState->eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
		pxState->uIsAvailable = 1U;
	}
	return RSRX_TRANSPORT_STATUS_OK;
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

static void vApplicationDataNotify(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	const rsrx_application_data_indication_t * pxIndication)
{
	test_application_context_t * pxContext = (test_application_context_t *)pvContext;
	(void)pxReport;
	pxContext->uCallCount++;
	pxContext->xLastIndication = *pxIndication;
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

static void vFillConfig(
	rsrx_session_config_t * pxConfig,
	test_transport_context_t * pxTransport,
	test_clock_context_t * pxClock,
	test_timer_context_t * pxTimer,
	test_diagnostics_context_t * pxDiagnostics,
	test_application_context_t * pxApplication,
	test_counter_t * pxApiCounter,
	test_counter_t * pxLifecycleCounter,
	const uint8_t * puPayload,
	size_t xPayloadLength)
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
	pxConfig->puFramePayload = puPayload;
	pxConfig->xFramePayloadLength = xPayloadLength;
	pxConfig->uSupervisionIntervalNs = 200U;
	pxConfig->uRetransmissionIntervalNs = 300U;
	pxConfig->uDiagnosticFlushIntervalNs = 400U;
	pxConfig->uBusyRejectErrorThreshold = 0U;
	pxConfig->pvApplicationDataContext = pxApplication;
	pxConfig->pfApplicationData = vApplicationDataNotify;
	pxConfig->pvApiCallbackContext = pxApiCounter;
	pxConfig->pfApiNotification = vApiNotify;
	pxConfig->pvLifecycleCallbackContext = pxLifecycleCounter;
	pxConfig->pfLifecycleNotification = vLifecycleNotify;
}

static void vPrepareEstablishedSession(
	rsrx_session_t * pxSession,
	rsrx_session_config_t * pxConfig,
	const rsrx_orchestrator_report_t ** ppxReport,
	test_transport_context_t * pxTransport,
	test_clock_context_t * pxClock,
	test_timer_context_t * pxTimer,
	test_diagnostics_context_t * pxDiagnostics,
	test_application_context_t * pxApplication,
	test_counter_t * pxApiCounter,
	test_counter_t * pxLifecycleCounter,
	const uint8_t * puPayload,
	size_t xPayloadLength)
{
	vFillConfig(
		pxConfig,
		pxTransport,
		pxClock,
		pxTimer,
		pxDiagnostics,
		pxApplication,
		pxApiCounter,
		pxLifecycleCounter,
		puPayload,
		xPayloadLength);
	vAssertTrue(rsrx_session_init(pxSession, pxConfig) == RSRX_STATUS_OK, "session init");
	vAssertTrue(rsrx_session_start(pxSession, ppxReport) == RSRX_STATUS_OK, "session start");
	vAssertTrue(rsrx_session_connect(pxSession, ppxReport) == RSRX_STATUS_OK, "session connect");
	vAssertTrue(rsrx_session_process_event(pxSession, RSRX_EVENT_HANDSHAKE_SUCCESS, ppxReport) == RSRX_STATUS_OK, "handshake success");
	vAssertTrue(rsrx_session_get_state(pxSession) == RSRX_STATE_ESTABLISHED, "session established");
}

static void vTestSessionStartupAndConnect(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[3] = { 0x01U, 0x02U, 0x03U };

	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xApplication, &xApiCounter, &xLifecycleCounter, auPayload, sizeof(auPayload));

	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "session init");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_UNINITIALIZED, "initial session state");

	vAssertTrue(rsrx_session_start(&xSession, &pxReport) == RSRX_STATUS_OK, "session start");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_INIT_COMPLETED, "start reason");
	vAssertTrue(xApiCounter.uCallCount == 1U, "api notified on start");
	vAssertTrue(xDiagnostics.uCallCount == 1U, "diagnostics written on start");

	vAssertTrue(rsrx_session_connect(&xSession, &pxReport) == RSRX_STATUS_OK, "session connect");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_CONNECTING, "connecting state");
	vAssertTrue(xTransport.uSendCount == 1U, "transport send count");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_CONNECT_REQUESTED, "transport reason on connect");
	vAssertTrue(xTimer.uCallCount == 1U, "timer command count");
	vAssertTrue(xApiCounter.uCallCount == 2U, "api notified on connect");
	vAssertTrue(xDiagnostics.uCallCount == 1U, "no new diagnostics on connect");
}

static void vTestSessionDisconnectPath(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 500U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[2] = { 0xAAU, 0xBBU };

	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xApplication, &xApiCounter, &xLifecycleCounter, auPayload, sizeof(auPayload));
	(void)rsrx_session_init(&xSession, &xConfig);
	(void)rsrx_session_start(&xSession, &pxReport);
	(void)rsrx_session_connect(&xSession, &pxReport);
	(void)rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxReport);

	vAssertTrue(rsrx_session_disconnect(&xSession, &pxReport) == RSRX_STATUS_OK, "session disconnect");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_DISCONNECT_REQUESTED, "disconnect reason");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "disconnect state");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "lifecycle callback count");
	vAssertTrue(xApiCounter.uCallCount >= 3U, "api callback count after disconnect");
	vAssertTrue(
		(xTransport.uSendCount == 1U) || (xTransport.uSendCount == 2U),
		"transport send count after disconnect");
	if(xTransport.uSendCount == 2U)
	{
		vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_DISCONNECT_REQUESTED, "disconnect transport reason");
	}
}

static void vTestSessionInboundHeartbeatPath(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 700U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[2] = { 0x11U, 0x22U };

	vPrepareEstablishedSession(
		&xSession,
		&xConfig,
		&pxReport,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auPayload,
		sizeof(auPayload));

	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_VALID_HEARTBEAT, &pxReport) == RSRX_STATUS_OK, "heartbeat event");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "heartbeat keeps established");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_HEARTBEAT_ACCEPTED, "heartbeat reason");
	vAssertTrue(pxReport->uDispatchedActionCount == 2U, "heartbeat dispatched actions");
	vAssertTrue(xTransport.uSendCount == 1U, "heartbeat does not send transport payload");
	vAssertTrue(xTimer.uCallCount == 3U, "heartbeat timer restart");
	vAssertTrue(xTimer.xLastCommand.eCommandType == RSRX_TIMER_COMMAND_RESTART, "heartbeat timer command");
	vAssertTrue(xDiagnostics.uCallCount == 3U, "heartbeat diagnostic count");
	vAssertTrue(xApiCounter.uCallCount == 3U, "heartbeat no api notify");
}

static void vTestSessionInboundDataPath(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 800U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[3] = { 0x21U, 0x22U, 0x23U };

	vPrepareEstablishedSession(
		&xSession,
		&xConfig,
		&pxReport,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auPayload,
		sizeof(auPayload));
	rsrx_transport_adapter_record_inbound_message(
		&xSession.xTransportAdapter,
		&(const rsrx_decoded_message_t){
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_EVENT_VALID_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			5U,
			4U,
			{ 0x21U, 0x22U, 0x23U },
			3U });

	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_VALID_DATA, &pxReport) == RSRX_STATUS_OK, "data event");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "data keeps established");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_DATA_ACCEPTED, "data reason");
	vAssertTrue(pxReport->uDispatchedActionCount == 3U, "data dispatched actions");
	vAssertTrue(xTransport.uSendCount == 1U, "data delivery does not send outbound transport");
	vAssertTrue(xApplication.uCallCount == 1U, "application data callback count");
	vAssertTrue(xApplication.xLastIndication.xPayloadLength == 3U, "application data payload length");
	vAssertTrue(xApplication.xLastIndication.puPayload != (const uint8_t *)0, "application data payload pointer");
	vAssertTrue(xApplication.xLastIndication.puPayload[0] == 0x21U, "application data payload byte 0");
	vAssertTrue(xApplication.xLastIndication.eReason == RSRX_REASON_DATA_ACCEPTED, "application data reason");
	vAssertTrue(xApplication.xLastIndication.uSequenceNumber == 5U, "application data sequence");
	vAssertTrue(xApplication.xLastIndication.uConfirmationNumber == 4U, "application data confirmation");
	vAssertTrue(xTimer.uCallCount == 3U, "data timer restart");
	vAssertTrue(xDiagnostics.uCallCount == 3U, "data diagnostic count");
	vAssertTrue(xApiCounter.uCallCount == 3U, "data no api notify");
}

static void vTestSessionOutboundApplicationDataPath(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 850U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auFramePayload[3] = { 0x21U, 0x22U, 0x23U };
	static const uint8_t auDataPayload[4] = { 0x61U, 0x62U, 0x63U, 0x64U };

	vPrepareEstablishedSession(
		&xSession,
		&xConfig,
		&pxReport,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auFramePayload,
		sizeof(auFramePayload));
	pxTelemetry = rsrx_session_get_outbound_telemetry(&xSession);
	vAssertTrue(pxTelemetry != (const rsrx_outbound_send_telemetry_t *)0, "outbound application telemetry available");

	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_OK,
		"application data send");
	vAssertTrue(xTransport.uSendCount == 2U, "outbound application send count");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_APPLICATION_DATA_REQUESTED, "outbound application send reason");
	vAssertTrue(xTransport.xLastRequest.xPayloadLength == (D_RSRX_CODEC_HEADER_BYTES + sizeof(auDataPayload)), "outbound application encoded length");
	vAssertTrue(xTransport.xLastRequest.puPayload[0] == (uint8_t)RSRX_MESSAGE_TYPE_DATA, "outbound application message type");
	vAssertTrue(xTransport.xLastRequest.puPayload[1] == (uint8_t)RSRX_REASON_APPLICATION_DATA_REQUESTED, "outbound application encoded reason");
	vAssertTrue(xTransport.xLastRequest.puPayload[7] == 0x02U, "outbound application sequence");
	vAssertTrue(xTransport.xLastRequest.puPayload[11] == 0x00U, "outbound application confirmation");
	vAssertTrue(xTransport.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auDataPayload[0], "outbound application payload copied");
	vAssertTrue(xApplication.uCallCount == 0U, "outbound application send does not trigger inbound callback");
	vAssertTrue(pxTelemetry->uAcceptedSendCount == 2U, "outbound application accepted telemetry");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 0U, "outbound application queued telemetry before queue");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 0U, "outbound application busy telemetry before reject");
	vAssertTrue(xApiCounter.uCallCount == 3U, "outbound application api count before reject");
	vAssertTrue(xDiagnostics.uCallCount == 2U, "outbound application diagnostic count before reject");
	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_OK,
		"application data send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 1U, "outbound application queued telemetry after queue");
	vAssertTrue(xApiCounter.uCallCount == 3U, "outbound application api count after queue");
	vAssertTrue(xDiagnostics.uCallCount == 2U, "outbound application diagnostic count after queue");
	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_REJECTED,
		"application data send overflow reject");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 1U, "outbound application busy telemetry after reject");
	vAssertTrue(pxTelemetry->uQueueOverflowRejectCount == 1U, "outbound application queue overflow telemetry after reject");
	vAssertTrue(pxTelemetry->eLastSendStatus == RSRX_TRANSPORT_STATUS_UNAVAILABLE, "outbound application last send status telemetry");
	vAssertTrue(xApiCounter.uCallCount == 4U, "outbound application api count after reject");
	vAssertTrue(xDiagnostics.uCallCount == 3U, "outbound application diagnostic count after reject");
	vAssertTrue(xDiagnostics.xLastRecord.eStatus == RSRX_STATUS_REJECTED, "outbound application reject diagnostic status");
	vAssertTrue(xDiagnostics.xLastRecord.eReason == RSRX_REASON_APPLICATION_DATA_REQUESTED, "outbound application reject diagnostic reason");
	vAssertTrue(xDiagnostics.xLastRecord.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "outbound application reject diagnostic code");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eStatus == RSRX_STATUS_REJECTED, "outbound application reject report status");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eReason == RSRX_REASON_APPLICATION_DATA_REQUESTED, "outbound application reject report reason");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "outbound application reject report diagnostic");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "outbound application busy streak one");
	vAssertTrue(pxTelemetry->uMaxConsecutiveBusyRejectedSendCount == 1U, "outbound application busy max one");
	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_REJECTED,
		"application data second overflow guard");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 2U, "outbound application busy telemetry after second reject");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 2U, "outbound application busy streak two");
	vAssertTrue(pxTelemetry->uMaxConsecutiveBusyRejectedSendCount == 2U, "outbound application busy max two");
	vAssertTrue(xApiCounter.uCallCount == 5U, "outbound application api count after second reject");
	vAssertTrue(xDiagnostics.uCallCount == 4U, "outbound application diagnostic count after second reject");
}

static void vTestSessionOutboundApplicationBusyRejectThreshold(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 860U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auFramePayload[3] = { 0x24U, 0x25U, 0x26U };
	static const uint8_t auDataPayload[2] = { 0x71U, 0x72U };

	vPrepareEstablishedSession(
		&xSession,
		&xConfig,
		&pxReport,
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
	xSession.uBusyRejectErrorThreshold = 2U;
	pxTelemetry = rsrx_session_get_outbound_telemetry(&xSession);

	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_OK,
		"busy reject threshold priming send");
	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_OK,
		"busy reject threshold queued send");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 1U, "busy reject threshold queued telemetry");
	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_REJECTED,
		"busy reject threshold first reject");
	vAssertTrue(xDiagnostics.xLastRecord.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "busy reject threshold first diagnostic warning");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eDiagnostic == RSRX_DIAG_WARN_REJECTED_EVENT, "busy reject threshold first report warning");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "busy reject threshold streak one");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 0U, "busy reject threshold escalation count before threshold");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 0U, "busy reject threshold escalation latch before threshold");

	vAssertTrue(
		rsrx_session_send_application_data(
			&xSession,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_STATUS_REJECTED,
		"busy reject threshold second reject");
	vAssertTrue(xDiagnostics.xLastRecord.eDiagnostic == RSRX_DIAG_ERROR_INTERFACE, "busy reject threshold second diagnostic error");
	vAssertTrue(xDiagnostics.xLastRecord.eSeverity == RSRX_LOG_SEVERITY_ERROR, "busy reject threshold second severity error");
	vAssertTrue(xApiCounter.xLastReport.xTransition.eDiagnostic == RSRX_DIAG_ERROR_INTERFACE, "busy reject threshold second report error");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 2U, "busy reject threshold streak two");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 1U, "busy reject threshold escalation count after threshold");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 1U, "busy reject threshold escalation latch after threshold");
}

static void vTestSessionRetransmissionPath(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 900U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[4] = { 0x31U, 0x32U, 0x33U, 0x34U };

	vPrepareEstablishedSession(
		&xSession,
		&xConfig,
		&pxReport,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auPayload,
		sizeof(auPayload));

	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_SEQUENCE_GAP_DETECTED, &pxReport) == RSRX_STATUS_OK, "sequence gap event");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "retransmission pending");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "sequence gap reason");
	vAssertTrue(pxReport->uDispatchedActionCount == 3U, "sequence gap dispatched actions");
	vAssertTrue(xTransport.uSendCount == 2U, "retransmission request sent");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "retransmission reason");
	vAssertTrue(xTimer.uCallCount == 2U, "sequence gap no timer restart");
	vAssertTrue(xDiagnostics.uCallCount == 3U, "sequence gap diagnostic count");
	vAssertTrue(xApiCounter.uCallCount == 4U, "sequence gap api notify");

	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_RECOVERY_SUCCESS, &pxReport) == RSRX_STATUS_OK, "recovery success event");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "recovery returns established");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "recovery reason");
	vAssertTrue(pxReport->uDispatchedActionCount == 4U, "recovery dispatched actions");
	vAssertTrue(xTimer.uCallCount == 3U, "recovery timer restart");
	vAssertTrue(xDiagnostics.uCallCount == 4U, "recovery diagnostic count");
	vAssertTrue(xApiCounter.uCallCount == 5U, "recovery api notify");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "recovery lifecycle action");
}

static void vTestSessionSupervisionTimerExpiry(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[2] = { 0x41U, 0x42U };

	vPrepareEstablishedSession(
		&xSession,
		&xConfig,
		&pxReport,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auPayload,
		sizeof(auPayload));

	vAssertTrue(rsrx_session_process_timer_expiry(&xSession, RSRX_TIMER_EXPIRY_SUPERVISION, &pxReport) == RSRX_STATUS_REJECTED, "supervision timeout status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "supervision timeout state");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_TIMEOUT_EXPIRED, "supervision timeout reason");
	vAssertTrue(pxReport->uDispatchedActionCount == 4U, "supervision timeout actions");
	vAssertTrue(xTransport.uSendCount == 2U, "supervision timeout disconnect sent");
	vAssertTrue(xApiCounter.uCallCount == 4U, "supervision timeout api notify");
	vAssertTrue(xDiagnostics.uCallCount == 3U, "supervision timeout diagnostic");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "supervision timeout lifecycle");
}

static void vTestSessionRetransmissionTimerExpiry(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 1100U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[2] = { 0x51U, 0x52U };

	vPrepareEstablishedSession(
		&xSession,
		&xConfig,
		&pxReport,
		&xTransport,
		&xClock,
		&xTimer,
		&xDiagnostics,
		&xApplication,
		&xApiCounter,
		&xLifecycleCounter,
		auPayload,
		sizeof(auPayload));
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_SEQUENCE_GAP_DETECTED, &pxReport) == RSRX_STATUS_OK, "enter retransmission pending");

	vAssertTrue(rsrx_session_process_timer_expiry(&xSession, RSRX_TIMER_EXPIRY_RETRANSMISSION, &pxReport) == RSRX_STATUS_REJECTED, "retransmission timeout status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "retransmission timeout state");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_RETRANSMISSION_FAILED, "retransmission timeout reason");
	vAssertTrue(pxReport->uDispatchedActionCount == 4U, "retransmission timeout actions");
	vAssertTrue(
		(xTransport.uSendCount == 2U) || (xTransport.uSendCount == 3U),
		"retransmission timeout transport send count");
	vAssertTrue(xApiCounter.uCallCount == 5U, "retransmission timeout api notify");
	vAssertTrue(xDiagnostics.uCallCount == 4U, "retransmission timeout diagnostic");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "retransmission timeout lifecycle");
}

static void vTestInvalidArguments(void)
{
	rsrx_session_t xSession = { 0 };
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 100U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[1] = { 0x01U };

	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xApplication, &xApiCounter, &xLifecycleCounter, auPayload, sizeof(auPayload));
	xConfig.uSupervisionIntervalNs = 0U;

	vAssertTrue(rsrx_session_init((rsrx_session_t *)0, (const rsrx_session_config_t *)0) == RSRX_STATUS_INVALID_ARGUMENT, "null session init");
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_INVALID_ARGUMENT, "invalid config rejected");
	vAssertTrue(rsrx_session_start(&xSession, &pxReport) == RSRX_STATUS_INVALID_ARGUMENT, "start before init");
	vAssertTrue(rsrx_session_send_application_data((rsrx_session_t *)0, auPayload, sizeof(auPayload)) == RSRX_STATUS_INVALID_ARGUMENT, "null session send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_INVALID_ARGUMENT, "send before init");
	vAssertTrue(rsrx_session_process_timer_expiry(&xSession, RSRX_TIMER_EXPIRY_INVALID, &pxReport) == RSRX_STATUS_INVALID_ARGUMENT, "invalid timer source");
	vAssertTrue(rsrx_session_process_timer_expiry(&xSession, RSRX_TIMER_EXPIRY_DIAGNOSTIC_FLUSH, &pxReport) == RSRX_STATUS_INVALID_ARGUMENT, "unsupported timer source");
	vAssertTrue(rsrx_session_get_state((const rsrx_session_t *)0) == RSRX_STATE_INVALID, "get state null");
	vAssertTrue(rsrx_session_reset((rsrx_session_t *)0) == RSRX_STATUS_INVALID_ARGUMENT, "reset null");
}

static void vTestSessionOutboundApplicationDataStateGuards(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	const rsrx_orchestrator_report_t * pxReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 1200U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_application_context_t xApplication = { { (const uint8_t *)0, 0U, RSRX_REASON_NONE, 0U, 0U }, 0U };
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[2] = { 0x71U, 0x72U };

	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xApplication, &xApiCounter, &xLifecycleCounter, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "send guard session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxReport) == RSRX_STATUS_OK, "send guard session start");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_INVALID_STATE, "send guard invalid state");
	vAssertTrue(rsrx_session_connect(&xSession, &pxReport) == RSRX_STATUS_OK, "send guard session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxReport) == RSRX_STATUS_OK, "send guard establish");
	vAssertTrue(rsrx_session_send_application_data(&xSession, (const uint8_t *)0, sizeof(auPayload)) == RSRX_STATUS_INVALID_ARGUMENT, "send guard null payload");
}

int main(void)
{
	vTestSessionStartupAndConnect();
	vTestSessionDisconnectPath();
	vTestSessionInboundHeartbeatPath();
	vTestSessionInboundDataPath();
	vTestSessionOutboundApplicationDataPath();
	vTestSessionOutboundApplicationBusyRejectThreshold();
	vTestSessionRetransmissionPath();
	vTestSessionSupervisionTimerExpiry();
	vTestSessionRetransmissionTimerExpiry();
	vTestInvalidArguments();
	vTestSessionOutboundApplicationDataStateGuards();

	(void)printf("rsrx_api_test: all tests passed\n");

	return EXIT_SUCCESS;
}
