#include <stdio.h>
#include <stdlib.h>

#include "rsrx_transport_supervisor.h"

typedef struct
{
	rsrx_monotonic_time_ns_t uNowNs;
} test_clock_context_t;

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

typedef struct
{
	uint32_t uApiCount;
	uint32_t uLifecycleCount;
} test_callback_context_t;

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
	test_callback_context_t * pxContext = (test_callback_context_t *)pvContext;
	(void)pxReport;
	pxContext->uApiCount++;
}

static void vLifecycleNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport, rsrx_action_t eAction, uint32_t uActionIndex)
{
	test_callback_context_t * pxContext = (test_callback_context_t *)pvContext;
	(void)pxReport;
	(void)eAction;
	(void)uActionIndex;
	pxContext->uLifecycleCount++;
}

static rsrx_codec_status_t eDecodeFrame(const rsrx_transport_frame_t * pxFrame, rsrx_decoded_message_t * pxMessage)
{
	if((pxFrame == (const rsrx_transport_frame_t *)0) ||
		(pxMessage == (rsrx_decoded_message_t *)0))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	pxMessage->eMessageType = RSRX_MESSAGE_TYPE_CONNECT_RESPONSE;
	pxMessage->eSuggestedEvent = RSRX_EVENT_HANDSHAKE_SUCCESS;
	pxMessage->eReason = RSRX_REASON_HANDSHAKE_COMPLETED;
	pxMessage->uSequenceNumber = 1U;
	pxMessage->uConfirmationNumber = 1U;
	pxMessage->xPayloadLength = pxFrame->xPayloadLength;
	return RSRX_CODEC_STATUS_OK;
}

static void vFillConfig(
	rsrx_session_config_t * pxConfig,
	test_transport_context_t * pxTransport,
	test_clock_context_t * pxClock,
	test_timer_context_t * pxTimer,
	test_diagnostics_context_t * pxDiagnostics,
	test_callback_context_t * pxCallbacks,
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
	pxConfig->uSupervisionIntervalNs = 100U;
	pxConfig->uRetransmissionIntervalNs = 200U;
	pxConfig->uDiagnosticFlushIntervalNs = 300U;
	pxConfig->pvApiCallbackContext = pxCallbacks;
	pxConfig->pfApiNotification = vApiNotify;
	pxConfig->pvLifecycleCallbackContext = pxCallbacks;
	pxConfig->pfLifecycleNotification = vLifecycleNotify;
}

static void vTestSupervisorInboundHandshakePath(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[3] = { 0x01U, 0x02U, 0x03U };

	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session connect");

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "process frame");
	vAssertTrue(pxSupervisorReport->xLastMessage.eSuggestedEvent == RSRX_EVENT_HANDSHAKE_SUCCESS, "decoded suggested event");
	vAssertTrue(pxSupervisorReport->pxLastReport != (const rsrx_orchestrator_report_t *)0, "session report available");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_HANDSHAKE_COMPLETED, "handover reason");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "session established");
}

static void vTestSupervisorInvalidArguments(void)
{
	rsrx_transport_supervisor_context_t xSupervisor = { 0 };
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	vAssertTrue(rsrx_transport_supervisor_init((rsrx_transport_supervisor_context_t *)0, (rsrx_session_t *)0, (const rsrx_codec_port_t *)0) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "null supervisor init");
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, (const rsrx_transport_frame_t *)0, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "process frame invalid args");
}

int main(void)
{
	vTestSupervisorInboundHandshakePath();
	vTestSupervisorInvalidArguments();

	(void)printf("rsrx_transport_supervisor_test: all tests passed\n");

	return EXIT_SUCCESS;
}
