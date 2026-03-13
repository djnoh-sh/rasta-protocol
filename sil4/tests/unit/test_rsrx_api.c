#include <stdio.h>
#include <stdlib.h>

#include "rsrx_api.h"

typedef struct
{
	rsrx_monotonic_time_ns_t uNowNs;
} test_clock_context_t;

typedef struct
{
	uint32_t uCallCount;
} test_counter_t;

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

static void vFillConfig(
	rsrx_session_config_t * pxConfig,
	test_transport_context_t * pxTransport,
	test_clock_context_t * pxClock,
	test_timer_context_t * pxTimer,
	test_diagnostics_context_t * pxDiagnostics,
	test_counter_t * pxApiCounter,
	test_counter_t * pxLifecycleCounter,
	const uint8_t * puPayload,
	size_t xPayloadLength)
{
	pxConfig->xTransportPort.pvContext = pxTransport;
	pxConfig->xTransportPort.pfSend = eTransportSend;
	pxConfig->xTransportPort.pfReceive = eTransportReceive;
	pxConfig->xTransportPort.pfQueryChannel = eTransportQuery;
	pxConfig->xPlatformPorts.xClock.pvContext = pxClock;
	pxConfig->xPlatformPorts.xClock.pfNow = eClockNow;
	pxConfig->xPlatformPorts.xTimer.pvContext = pxTimer;
	pxConfig->xPlatformPorts.xTimer.pfCommand = eTimerCommand;
	pxConfig->xPlatformPorts.xDiagnostics.pvContext = pxDiagnostics;
	pxConfig->xPlatformPorts.xDiagnostics.pfWrite = eDiagnosticWrite;
	pxConfig->eDefaultChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->puFramePayload = puPayload;
	pxConfig->xFramePayloadLength = xPayloadLength;
	pxConfig->uSupervisionIntervalNs = 200U;
	pxConfig->uRetransmissionIntervalNs = 300U;
	pxConfig->uDiagnosticFlushIntervalNs = 400U;
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
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[3] = { 0x01U, 0x02U, 0x03U };

	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xApiCounter, &xLifecycleCounter, auPayload, sizeof(auPayload));

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
	test_counter_t xApiCounter = { 0U };
	test_counter_t xLifecycleCounter = { 0U };
	static const uint8_t auPayload[2] = { 0xAAU, 0xBBU };

	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xApiCounter, &xLifecycleCounter, auPayload, sizeof(auPayload));
	(void)rsrx_session_init(&xSession, &xConfig);
	(void)rsrx_session_start(&xSession, &pxReport);
	(void)rsrx_session_connect(&xSession, &pxReport);
	(void)rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxReport);

	vAssertTrue(rsrx_session_disconnect(&xSession, &pxReport) == RSRX_STATUS_OK, "session disconnect");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_DISCONNECT_REQUESTED, "disconnect reason");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "disconnect state");
	vAssertTrue(xLifecycleCounter.uCallCount == 1U, "lifecycle callback count");
	vAssertTrue(xApiCounter.uCallCount >= 3U, "api callback count after disconnect");
	vAssertTrue(xTransport.uSendCount >= 2U, "transport send count after disconnect");
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
		&xApiCounter,
		&xLifecycleCounter,
		auPayload,
		sizeof(auPayload));

	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_VALID_DATA, &pxReport) == RSRX_STATUS_OK, "data event");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "data keeps established");
	vAssertTrue(pxReport->xTransition.eReason == RSRX_REASON_DATA_ACCEPTED, "data reason");
	vAssertTrue(pxReport->uDispatchedActionCount == 3U, "data dispatched actions");
	vAssertTrue(xTransport.uSendCount == 2U, "data delivery routed through transport executor");
	vAssertTrue(xTransport.xLastRequest.eReason == RSRX_REASON_DATA_ACCEPTED, "data delivery reason");
	vAssertTrue(xTimer.uCallCount == 3U, "data timer restart");
	vAssertTrue(xDiagnostics.uCallCount == 3U, "data diagnostic count");
	vAssertTrue(xApiCounter.uCallCount == 3U, "data no api notify");
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

static void vTestInvalidArguments(void)
{
	rsrx_session_t xSession = { 0 };
	const rsrx_orchestrator_report_t * pxReport;
	vAssertTrue(rsrx_session_init((rsrx_session_t *)0, (const rsrx_session_config_t *)0) == RSRX_STATUS_INVALID_ARGUMENT, "null session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxReport) == RSRX_STATUS_INVALID_ARGUMENT, "start before init");
	vAssertTrue(rsrx_session_get_state((const rsrx_session_t *)0) == RSRX_STATE_INVALID, "get state null");
	vAssertTrue(rsrx_session_reset((rsrx_session_t *)0) == RSRX_STATUS_INVALID_ARGUMENT, "reset null");
}

int main(void)
{
	vTestSessionStartupAndConnect();
	vTestSessionDisconnectPath();
	vTestSessionInboundHeartbeatPath();
	vTestSessionInboundDataPath();
	vTestSessionRetransmissionPath();
	vTestInvalidArguments();

	(void)printf("rsrx_api_test: all tests passed\n");

	return EXIT_SUCCESS;
}
