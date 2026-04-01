#include <stdio.h>
#include <stdlib.h>

#include "rsrx_channel_manager.h"
#include "rsrx_transport_supervisor.h"
#include "rsrx_codec.h"

typedef struct
{
	rsrx_monotonic_time_ns_t uNowNs;
} test_clock_context_t;

typedef struct
{
	rsrx_transport_send_request_t xLastRequest;
	rsrx_transport_frame_t xNextReceiveFrame;
	rsrx_transport_status_t eReceiveStatus;
	rsrx_transport_status_t eQueryStatus;
	rsrx_transport_frame_t axReceiveFrames[4];
	rsrx_transport_status_t aeReceiveStatuses[4];
	uint32_t uReceiveScriptCount;
	uint32_t uReceiveScriptIndex;
	uint32_t uPrimaryAvailable;
	uint32_t uSecondaryAvailable;
	uint32_t uSendCount;
	uint32_t uReceiveCount;
	uint32_t uQueryCount;
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
	uint32_t uApplicationCount;
	uint32_t uLifecycleCount;
} test_callback_context_t;

typedef struct
{
	rsrx_codec_status_t eStatus;
	rsrx_decoded_message_t xMessage;
	rsrx_codec_status_t aeStatuses[4];
	rsrx_decoded_message_t axMessages[4];
	uint32_t uScriptCount;
	uint32_t uScriptIndex;
	uint32_t uCallCount;
} test_codec_context_t;

static test_codec_context_t g_xCodecContext;

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

	*pxFrame = pxContext->xNextReceiveFrame;
	return pxContext->eReceiveStatus;
}

static rsrx_transport_status_t eTransportQuery(void * pvContext, rsrx_transport_channel_state_t * pxState)
{
	test_transport_context_t * pxContext = (test_transport_context_t *)pvContext;

	if(pxContext == (test_transport_context_t *)0)
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

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
	return pxContext->eQueryStatus;
}

static void vApiNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport)
{
	test_callback_context_t * pxContext = (test_callback_context_t *)pvContext;
	(void)pxReport;
	pxContext->uApiCount++;
}

static void vApplicationDataNotify(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	const rsrx_application_data_indication_t * pxIndication)
{
	test_callback_context_t * pxContext = (test_callback_context_t *)pvContext;
	(void)pxReport;
	(void)pxIndication;
	pxContext->uApplicationCount++;
}

static void vLifecycleNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport, rsrx_action_t eAction, uint32_t uActionIndex)
{
	test_callback_context_t * pxContext = (test_callback_context_t *)pvContext;
	(void)pxReport;
	(void)eAction;
	(void)uActionIndex;
	pxContext->uLifecycleCount++;
}

static void vSetCodecBehavior(
	rsrx_codec_status_t eStatus,
	rsrx_message_type_t eMessageType,
	rsrx_event_t eSuggestedEvent,
	rsrx_reason_code_t eReason,
	uint32_t uSequenceNumber,
	uint32_t uConfirmationNumber)
{
	g_xCodecContext.eStatus = eStatus;
	g_xCodecContext.uCallCount = 0U;
	g_xCodecContext.uScriptCount = 0U;
	g_xCodecContext.uScriptIndex = 0U;
	g_xCodecContext.xMessage.eMessageType = eMessageType;
	g_xCodecContext.xMessage.eSuggestedEvent = eSuggestedEvent;
	g_xCodecContext.xMessage.eReason = eReason;
	g_xCodecContext.xMessage.uSequenceNumber = uSequenceNumber;
	g_xCodecContext.xMessage.uConfirmationNumber = uConfirmationNumber;
	g_xCodecContext.xMessage.xPayloadLength = 0U;
}

static void vSetCodecScript(
	const rsrx_codec_status_t * peStatuses,
	const rsrx_decoded_message_t * pxMessages,
	uint32_t uScriptCount)
{
	uint32_t uIndex;

	g_xCodecContext.uCallCount = 0U;
	g_xCodecContext.uScriptCount = uScriptCount;
	g_xCodecContext.uScriptIndex = 0U;
	for(uIndex = 0U; uIndex < uScriptCount; ++uIndex)
	{
		g_xCodecContext.aeStatuses[uIndex] = peStatuses[uIndex];
		g_xCodecContext.axMessages[uIndex] = pxMessages[uIndex];
	}
}

static rsrx_codec_status_t eDecodeFrame(const rsrx_transport_frame_t * pxFrame, rsrx_decoded_message_t * pxMessage)
{
	if((pxFrame == (const rsrx_transport_frame_t *)0) ||
		(pxMessage == (rsrx_decoded_message_t *)0))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	g_xCodecContext.uCallCount++;
	if(g_xCodecContext.uScriptIndex < g_xCodecContext.uScriptCount)
	{
		*pxMessage = g_xCodecContext.axMessages[g_xCodecContext.uScriptIndex];
		g_xCodecContext.uScriptIndex++;
		pxMessage->xPayloadLength = pxFrame->xPayloadLength;
		return g_xCodecContext.aeStatuses[g_xCodecContext.uScriptIndex - 1U];
	}

	*pxMessage = g_xCodecContext.xMessage;
	pxMessage->xPayloadLength = pxFrame->xPayloadLength;
	return g_xCodecContext.eStatus;
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
	pxConfig->uSupervisionIntervalNs = 100U;
	pxConfig->uRetransmissionIntervalNs = 200U;
	pxConfig->uDiagnosticFlushIntervalNs = 300U;
	pxConfig->pvApplicationDataContext = pxCallbacks;
	pxConfig->pfApplicationData = vApplicationDataNotify;
	pxConfig->pvApiCallbackContext = pxCallbacks;
	pxConfig->pfApiNotification = vApiNotify;
	pxConfig->pvLifecycleCallbackContext = pxCallbacks;
	pxConfig->pfLifecycleNotification = vLifecycleNotify;
}

static void vInitTransportContext(
	test_transport_context_t * pxTransport,
	const uint8_t * puPayload,
	size_t xPayloadLength,
	rsrx_transport_event_type_t eEventType)
{
	pxTransport->xLastRequest.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxTransport->xLastRequest.puPayload = (const uint8_t *)0;
	pxTransport->xLastRequest.xPayloadLength = 0U;
	pxTransport->xLastRequest.eReason = RSRX_REASON_NONE;
	pxTransport->xNextReceiveFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxTransport->xNextReceiveFrame.puPayload = puPayload;
	pxTransport->xNextReceiveFrame.xPayloadLength = xPayloadLength;
	pxTransport->xNextReceiveFrame.eEventType = eEventType;
	pxTransport->eReceiveStatus = RSRX_TRANSPORT_STATUS_OK;
	pxTransport->eQueryStatus = RSRX_TRANSPORT_STATUS_OK;
	pxTransport->uReceiveScriptCount = 0U;
	pxTransport->uReceiveScriptIndex = 0U;
	pxTransport->uPrimaryAvailable = 1U;
	pxTransport->uSecondaryAvailable = 0U;
	pxTransport->uSendCount = 0U;
	pxTransport->uReceiveCount = 0U;
	pxTransport->uQueryCount = 0U;
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

static void vTestSupervisorInboundHandshakePath(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[3] = { 0x01U, 0x02U, 0x03U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "handshake path session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session connect");
	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_CONNECT_RESPONSE,
		RSRX_EVENT_HANDSHAKE_SUCCESS,
		RSRX_REASON_HANDSHAKE_COMPLETED,
		1U,
		1U);

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
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_HANDSHAKE_SUCCESS, "handover effective event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_OK, "handover session status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED, "handover decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_ACCEPTED, "handover decision class");
	vAssertTrue(pxSupervisorReport->uAcceptedDecisionCount == 1U, "handover accepted count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 1U, "processed frame count");
	vAssertTrue(g_xCodecContext.uCallCount == 1U, "codec called once");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "session established");
}

static void vTestSupervisorInvalidArguments(void)
{
	rsrx_transport_supervisor_context_t xSupervisor = { 0 };
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	vAssertTrue(rsrx_transport_supervisor_init((rsrx_transport_supervisor_context_t *)0, (rsrx_session_t *)0, (const rsrx_codec_port_t *)0) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "null supervisor init");
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, (const rsrx_transport_frame_t *)0, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "process frame invalid args");
}

static void vTestSupervisorDecodeFailure(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[2] = { 0x10U, 0x20U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "decode failure path session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session connect");
	vSetCodecBehavior(
		RSRX_CODEC_STATUS_DECODE_ERROR,
		RSRX_MESSAGE_TYPE_INVALID,
		RSRX_EVENT_INVALID,
		RSRX_REASON_NONE,
		0U,
		0U);

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_DECODE_FAILED, "decode failure status");
	vAssertTrue(pxSupervisorReport != (const rsrx_transport_supervisor_report_t *)0, "decode failure report");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 0U, "decode failure count");
	vAssertTrue(pxSupervisorReport->pxLastReport == (const rsrx_orchestrator_report_t *)0, "decode failure session report absent");
	vAssertTrue(pxSupervisorReport->xLastMessage.eSuggestedEvent == RSRX_EVENT_INVALID, "decode failure suggested event untouched");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_DECODE_FAILED, "decode failure decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_ERROR, "decode failure class");
	vAssertTrue(pxSupervisorReport->uErrorDecisionCount == 1U, "decode failure error count");
	vAssertTrue(g_xCodecContext.uCallCount == 1U, "decode failure codec call");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_CONNECTING, "decode failure leaves session state");
}

static void vTestSupervisorUnsupportedMessage(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[4] = { 0xABU, 0xCDU, 0xEFU, 0x01U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "unsupported message path session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "session connect");
	vSetCodecBehavior(
		RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
		RSRX_MESSAGE_TYPE_DIAGNOSTIC,
		RSRX_EVENT_INVALID,
		RSRX_REASON_PROTOCOL_ERROR_DETECTED,
		0U,
		0U);

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_DECODE_FAILED, "unsupported message status");
	vAssertTrue(pxSupervisorReport != (const rsrx_transport_supervisor_report_t *)0, "unsupported message report");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 0U, "unsupported message count");
	vAssertTrue(pxSupervisorReport->pxLastReport == (const rsrx_orchestrator_report_t *)0, "unsupported message session report absent");
	vAssertTrue(pxSupervisorReport->xLastMessage.eMessageType == RSRX_MESSAGE_TYPE_DIAGNOSTIC, "unsupported message type retained");
	vAssertTrue(pxSupervisorReport->xLastMessage.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "unsupported message reason retained");
	vAssertTrue(g_xCodecContext.uCallCount == 1U, "unsupported message codec call");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_CONNECTING, "unsupported message leaves session state");
}

static void vTestSupervisorSequenceGapDetection(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[3] = { 0x21U, 0x22U, 0x23U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "gap path session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "gap path session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "gap path session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "gap path establish");

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "gap path supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		1U,
		0U);
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "gap path first frame");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "gap path established remains");

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		0U);
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "gap path second frame");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "gap path retransmission pending");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_SEQUENCE_GAP_DETECTED, "gap path reason");
}

static void vTestSupervisorStaleSequenceProtocolError(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	rsrx_supervisor_status_t eStatus;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[2] = { 0x31U, 0x32U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "stale path session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "stale path session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "stale path session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "stale path establish");

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "stale path supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		1U,
		1U);
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "stale path first frame");

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		1U,
		1U);
	eStatus = rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport);
	vAssertTrue(eStatus == RSRX_SUPERVISOR_STATUS_OK, "stale path duplicate frame");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "stale path safe disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "stale path reason");
}

static void vTestSupervisorPollReceiveHandshake(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[3] = { 0x41U, 0x42U, 0x43U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "poll handshake session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll handshake session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll handshake session connect");

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_CONNECT_RESPONSE,
		RSRX_EVENT_HANDSHAKE_SUCCESS,
		RSRX_REASON_HANDSHAKE_COMPLETED,
		1U,
		1U);
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "poll handshake supervisor init");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "poll handshake receive");
	vAssertTrue(pxSupervisorReport->uPollCount == 1U, "poll handshake poll count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 1U, "poll handshake processed count");
	vAssertTrue(pxSupervisorReport->xLastChannelState.uIsAvailable == 1U, "poll handshake channel available");
	vAssertTrue(xTransport.uQueryCount == 1U, "poll handshake query count");
	vAssertTrue(xTransport.uReceiveCount == 1U, "poll handshake receive count");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "poll handshake established");
}

static void vTestSupervisorPollReceiveChannelDown(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x51U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	/* cppcheck-suppress redundantAssignment */
	/* cppcheck-suppress redundantAssignment */
	xTransport.uPrimaryAvailable = 0U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "poll down session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll down session start");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "poll down supervisor init");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN, "poll down status");
	vAssertTrue(pxSupervisorReport->uPollCount == 1U, "poll down poll count");
	vAssertTrue(xTransport.uReceiveCount == 0U, "poll down receive not called");
	vAssertTrue(pxSupervisorReport->xLastChannelState.uIsAvailable == 0U, "poll down channel unavailable");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN, "poll down decision");
}

static void vTestSupervisorPollReceiveNoFrame(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x61U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.eReceiveStatus = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "poll idle session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll idle session start");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "poll idle supervisor init");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_NO_FRAME, "poll idle status");
	vAssertTrue(pxSupervisorReport->uPollCount == 1U, "poll idle poll count");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 0U, "poll idle processed count");
	vAssertTrue(xTransport.uReceiveCount == 1U, "poll idle receive count");
}

static void vTestSupervisorPollReceiveErrorBudgeted(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x62U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	xTransport.eReceiveStatus = RSRX_TRANSPORT_STATUS_RX_ERROR;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "poll receive error session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll receive error session start");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "poll receive error supervisor init");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "poll receive error budgeted status");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "poll receive error count one");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "poll receive error reset count zero");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "poll receive error budgeted decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "poll receive error budgeted class");
}

static void vTestSupervisorPollReceiveErrorEscalatesAndResets(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x64U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	xTransport.eReceiveStatus = RSRX_TRANSPORT_STATUS_RX_ERROR;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "poll receive escalate session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll receive escalate session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll receive escalate session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "poll receive escalate establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "poll receive escalate supervisor init");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "poll receive escalate first status");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "poll receive escalate count one");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "poll receive escalate second status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "poll receive escalate safe disconnect");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "poll receive escalate reset count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "poll receive escalate reset telemetry");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "poll receive escalate effective event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "poll receive escalate session status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED, "poll receive escalate decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_ERROR, "poll receive escalate class");
	vAssertTrue(pxSupervisorReport->uErrorDecisionCount == 1U, "poll receive escalate error count");
}

static void vTestSupervisorPollReceiveRetryOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x65U };
	rsrx_transport_frame_t axFrames[4];
	rsrx_transport_status_t aeStatuses[4];

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "poll retry matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll retry matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "poll retry matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "poll retry matrix establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "poll retry matrix supervisor init");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auPayload;
	axFrames[0].xPayloadLength = sizeof(auPayload);
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1] = axFrames[0];
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	axFrames[2] = axFrames[0];
	axFrames[3] = axFrames[0];
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	aeStatuses[3] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 4U);

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "poll retry matrix first error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "poll retry matrix first error count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 0U, "poll retry matrix first error reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "poll retry matrix first error decision");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_NO_FRAME, "poll retry matrix no frame reset");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "poll retry matrix no frame clears count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "poll retry matrix no frame reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE, "poll retry matrix no frame decision");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "poll retry matrix second error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "poll retry matrix second error count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 1U, "poll retry matrix second error reset count stable");

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		1U,
		0U);
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "poll retry matrix success reset");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "poll retry matrix success clears count");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 2U, "poll retry matrix success reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED, "poll retry matrix success decision");
	vAssertTrue(xCallbacks.uApplicationCount == 1U, "poll retry matrix success callback");

	xTransport.eQueryStatus = RSRX_TRANSPORT_STATUS_OK;
	xTransport.uPrimaryAvailable = 0U;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN, "poll retry matrix channel gated");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "poll retry matrix channel gated count zero");
	vAssertTrue(pxSupervisorReport->uReceiveErrorBudgetResetCount == 2U, "poll retry matrix channel gated no extra reset");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN, "poll retry matrix channel gated decision");
}

static void vTestSupervisorChannelDownUsesFailover(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x63U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel failover session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel failover session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel failover session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "channel failover establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel failover supervisor init");

	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "budget scope primary available before second failover");
	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "channel failover priming send");
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover budget priming");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "channel failover budget primed");

	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover ignored");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel failover state retained");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel failover selected secondary");
	vAssertTrue(pxSupervisorReport->xLastChannelState.uIsAvailable == 1U, "channel failover selected available");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "channel failover budget reset");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_CHANNEL_DOWN, "channel failover budget update");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "channel failover budget reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED, "channel failover decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "channel failover class");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 2U, "channel failover ignored count");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel failover switch count");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 1U, "channel failover switch occurred");
	vAssertTrue(pxSupervisorReport->uFailoverSwitchCount == 1U, "channel failover failover count");
	vAssertTrue(pxSupervisorReport->uPreferredRecoverySwitchCount == 0U, "channel failover preferred recovery count");
	vAssertTrue(pxSupervisorReport->uNoOpRefreshCount == 0U, "channel failover no-op count");
	vAssertTrue(pxSupervisorReport->uHoldoffRefreshNoOpCount == 0U, "channel failover holdoff no-op count");
	vAssertTrue(pxSupervisorReport->uActiveRefreshNoOpCount == 0U, "channel failover active no-op count");
	vAssertTrue(pxSupervisorReport->eLastSwitchKind == RSRX_SUPERVISOR_SWITCH_KIND_FAILOVER, "channel failover switch kind");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_FAILOVER_CHANNEL_DOWN, "channel failover switch reason");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_DOWN, "channel failover trigger event");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel failover trigger channel");
	vAssertTrue(pxSupervisorReport->eLastSwitchFromChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel failover switch from primary");
	vAssertTrue(pxSupervisorReport->eLastSwitchToChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel failover switch to secondary");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "inactive channel send failure ignored");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "inactive channel send failure budget unchanged");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_NONE, "inactive channel send failure budget update none");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FAILURE_INACTIVE_CHANNEL_IGNORED, "inactive channel send failure decision");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 3U, "inactive channel send failure ignored count");
}

static void vTestSupervisorChannelUpRefreshesSelection(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x64U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel up refresh session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up refresh session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel up refresh session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "channel up refresh establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel up refresh supervisor init");

	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "budget scope primary available before second failover");
	vAssertTrue(xTransport.uSecondaryAvailable == 1U, "budget scope secondary available before second failover");
	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up refresh failover");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up refresh secondary selected");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up refresh ignored");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up refresh primary selected");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel up refresh decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "channel up refresh class");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 2U, "channel up refresh ignored count");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "channel up refresh switch count");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 1U, "channel up refresh switch occurred");
	vAssertTrue(pxSupervisorReport->uFailoverSwitchCount == 1U, "channel up refresh failover count retained");
	vAssertTrue(pxSupervisorReport->uPreferredRecoverySwitchCount == 1U, "channel up refresh preferred recovery count");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 1U, "channel up refresh immediate preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "channel up refresh holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "channel up refresh completed holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "channel up refresh bypass preferred recovery count");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_NONE, "channel up refresh holdoff cycle state");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "channel up refresh completed holdoff cycle kind");
	vAssertTrue(pxSupervisorReport->uNoOpRefreshCount == 0U, "channel up refresh no-op count retained");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 1U, "channel up refresh preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "channel up refresh non-preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 0U, "channel up refresh preferred-triggered no-op count retained");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "channel up refresh non-preferred-triggered no-op count retained");
	vAssertTrue(pxSupervisorReport->uHoldoffRefreshNoOpCount == 0U, "channel up refresh holdoff no-op count retained");
	vAssertTrue(pxSupervisorReport->uActiveRefreshNoOpCount == 0U, "channel up refresh active no-op count retained");
	vAssertTrue(pxSupervisorReport->eLastSwitchKind == RSRX_SUPERVISOR_SWITCH_KIND_PREFERRED_RECOVERY, "channel up refresh switch kind");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_IMMEDIATE, "channel up refresh switch reason");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_UP, "channel up refresh trigger event");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up refresh trigger channel");
	vAssertTrue(pxSupervisorReport->eLastSwitchFromChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel up refresh switch from secondary");
	vAssertTrue(pxSupervisorReport->eLastSwitchToChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up refresh switch to primary");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up refresh stale secondary failure ignored");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "channel up refresh stale secondary budget unchanged");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "channel up refresh priming send");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel up refresh primary failure first hit");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "channel up refresh primary failure budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel up refresh budget channel primary");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "channel up refresh primary budget update");
}

static void vTestSupervisorTransportSendFailed(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x71U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "send failed session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failed session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send failed session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "send failed establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "send failed supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "send failed priming send");
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failed first status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failed first keeps state");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "send failed first budget");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "send failed first budget update");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 0U, "send failed first reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED, "send failed first decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "send failed first class");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 1U, "send failed first ignored count");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "send failed second status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "send failed safe disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "send failed reason");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "send failed budget reset");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_ESCALATION, "send failed escalation budget update");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "send failed escalation reset count");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "send failed effective event");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "send failed session status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_REJECTED, "send failed escalation decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_REJECTED, "send failed escalation class");
	vAssertTrue(pxSupervisorReport->uRejectedDecisionCount == 1U, "send failed rejected count");
}

static void vTestSupervisorTransportSendCompletedIgnored(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x72U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "send complete session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send complete session start");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "send complete supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send complete ignored");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_INITIALIZED, "send complete leaves state");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "send complete budget reset");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_NONE, "send complete no budget change");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 0U, "send complete reset count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED, "send complete decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "send complete class");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 1U, "send complete ignored count");
}

static void vTestSupervisorTransportSendCompletedCorrelated(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x73U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "send complete correlated session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send complete correlated session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "send complete correlated session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "send complete correlated establish");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "send complete correlated first app send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "send complete correlated queue second send");
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xSession.xTransportAdapter) == 1U, "send complete correlated outstanding send set");
	pxTelemetry = rsrx_session_get_outbound_telemetry(&xSession);
	vAssertTrue(pxTelemetry != (const rsrx_outbound_send_telemetry_t *)0, "send complete correlated telemetry available");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 1U, "send complete correlated queued once");
	vAssertTrue(xSession.xTransportAdapter.uHasDeferredSend == 1U, "send complete correlated deferred present before feedback");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "send complete correlated supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send complete correlated ignored");
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xSession.xTransportAdapter) == 1U, "send complete correlated deferred redispatched to outstanding");
	vAssertTrue(pxTelemetry->uClearOnFeedbackCount == 1U, "send complete correlated feedback clear telemetry");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 1U, "send complete correlated deferred dispatch telemetry");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED, "send complete correlated decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "send complete correlated class");
	vAssertTrue(pxSupervisorReport->uOutstandingSendPresent == 1U, "send complete correlated report outstanding present");
	vAssertTrue(pxSupervisorReport->uDeferredSendPresent == 0U, "send complete correlated report deferred clear");
	vAssertTrue(pxSupervisorReport->uDeferredSendCount == 0U, "send complete correlated report deferred count");
	vAssertTrue(pxSupervisorReport->uQueuedSendCount == 1U, "send complete correlated report queued count");
	vAssertTrue(pxSupervisorReport->uMaxDeferredSendCount == 1U, "send complete correlated report max deferred count");
	vAssertTrue(pxSupervisorReport->uDeferredDispatchCount == 1U, "send complete correlated report dispatch count");
	vAssertTrue(pxSupervisorReport->uQueueOverflowRejectCount == 0U, "send complete correlated report no overflow");
	vAssertTrue(pxSupervisorReport->uBusyRejectedSendCount == 0U, "send complete correlated report no busy reject");
	vAssertTrue(pxSupervisorReport->uConsecutiveBusyRejectedSendCount == 0U, "send complete correlated report no busy streak");
	vAssertTrue(pxSupervisorReport->uMaxConsecutiveBusyRejectedSendCount == 0U, "send complete correlated report no busy max streak");
	vAssertTrue(pxSupervisorReport->uBusyRejectEscalationCount == 0U, "send complete correlated report no busy escalation");
	vAssertTrue(pxSupervisorReport->uLastBusyRejectEscalated == 0U, "send complete correlated report no busy latch");
}

static void vTestSupervisorReportExposesBusyRejectTelemetry(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[2] = { 0x91U, 0x92U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	xConfig.uBusyRejectErrorThreshold = 2U;
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "report busy telemetry session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "report busy telemetry session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "report busy telemetry session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "report busy telemetry establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "report busy telemetry supervisor init");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "report busy telemetry first send");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "report busy telemetry second queued");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "report busy telemetry third queued");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_REJECTED, "report busy telemetry first reject");
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_REJECTED, "report busy telemetry second reject");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "report busy telemetry refresh");

	vAssertTrue(pxSupervisorReport->uQueueOverflowRejectCount == 2U, "report busy telemetry overflow count");
	vAssertTrue(pxSupervisorReport->uBusyRejectedSendCount == 2U, "report busy telemetry busy reject count");
	vAssertTrue(pxSupervisorReport->uConsecutiveBusyRejectedSendCount == 0U, "report busy telemetry streak reset after clear");
	vAssertTrue(pxSupervisorReport->uMaxConsecutiveBusyRejectedSendCount == 2U, "report busy telemetry max streak");
	vAssertTrue(pxSupervisorReport->uBusyRejectEscalationCount == 1U, "report busy telemetry escalation count");
	vAssertTrue(pxSupervisorReport->uLastBusyRejectEscalated == 0U, "report busy telemetry latch reset after clear");
}

static void vTestSupervisorSendFailureBudgetResetsAfterSuccess(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[2] = { 0x74U, 0x75U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "budget reset session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "budget reset session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "budget reset session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "budget reset establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "budget reset supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "budget reset priming send");
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget reset first failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "budget reset counter one");

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		1U,
		0U);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "budget reset inbound success");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "budget reset cleared by success");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "budget reset second priming send");
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget reset failure after success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "budget reset state remains established");
}

static void vTestSupervisorBudgetScopeMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[2] = { 0x76U, 0x77U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "budget scope session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "budget scope session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "budget scope session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "budget scope establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "budget scope supervisor init");

	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "budget scope primary available before second failover");
	vAssertTrue(xTransport.uSecondaryAvailable == 1U, "budget scope secondary available before second failover");
	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget scope first failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "budget scope active secondary");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "budget scope secondary send");
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget scope secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "budget scope secondary send budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "budget scope secondary budget channel");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "budget scope secondary budget update");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget scope primary refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "budget scope active primary");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget scope stale secondary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "budget scope stale failure leaves send budget");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "budget scope stale failure leaves budget channel");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_NONE, "budget scope stale failure budget update none");

	rsrx_transport_adapter_clear_outstanding_send(&xSession.xTransportAdapter);
	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "budget scope primary send");
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget scope primary failure");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "budget scope primary send budget one after channel switch");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "budget scope primary budget channel");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_AND_INCREMENT_ON_CHANNEL_SWITCH, "budget scope primary budget update");
	vAssertTrue(pxSupervisorReport->uSendFailureBudgetResetCount == 1U, "budget scope send reset count one");

	xTransport.eReceiveStatus = RSRX_TRANSPORT_STATUS_RX_ERROR;
	xTransport.eQueryStatus = RSRX_TRANSPORT_STATUS_OK;
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget scope primary receive error");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "budget scope receive budget one");

	vAssertTrue(xTransport.uPrimaryAvailable == 1U, "budget scope primary available before second failover");
	vAssertTrue(xTransport.uSecondaryAvailable == 1U, "budget scope secondary available before second failover");
	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget scope second failover");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "budget scope active secondary again");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "budget scope channel down resets send budget");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "budget scope receive budget carryover");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "budget scope receive escalation");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "budget scope receive escalation event");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED, "budget scope receive escalation decision");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 0U, "budget scope receive escalation reset");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "budget scope safe disconnect");
}

static void vTestSupervisorSendFeedbackOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[2] = { 0x78U, 0x79U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "feedback ordering matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "feedback ordering matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "feedback ordering matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "feedback ordering matrix establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "feedback ordering matrix supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	xTransport.uPrimaryAvailable = 0U;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "feedback ordering matrix failover to secondary");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "feedback ordering matrix active secondary");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "feedback ordering matrix secondary send");
	vAssertTrue(rsrx_transport_adapter_get_outstanding_send_channel(&xSession.xTransportAdapter) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "feedback ordering matrix outstanding secondary");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "feedback ordering matrix primary refresh");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xSession.xChannelManager) == RSRX_TRANSPORT_CHANNEL_PRIMARY, "feedback ordering matrix active primary");
	vAssertTrue(rsrx_transport_adapter_get_outstanding_send_channel(&xSession.xTransportAdapter) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "feedback ordering matrix outstanding still secondary");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "feedback ordering matrix inactive secondary failure");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FAILURE_INACTIVE_CHANNEL_IGNORED, "feedback ordering matrix inactive secondary failure decision");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "feedback ordering matrix inactive secondary failure leaves budget zero");
	vAssertTrue(rsrx_transport_adapter_get_outstanding_send_channel(&xSession.xTransportAdapter) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "feedback ordering matrix inactive secondary failure keeps outstanding");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "feedback ordering matrix inactive secondary completion");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED, "feedback ordering matrix inactive secondary completion decision");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "feedback ordering matrix completion keeps budget zero");
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xSession.xTransportAdapter) == 0U, "feedback ordering matrix completion clears outstanding");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_NONE, "feedback ordering matrix completion budget update");

	vAssertTrue(rsrx_session_send_application_data(&xSession, auPayload, sizeof(auPayload)) == RSRX_STATUS_OK, "feedback ordering matrix primary send");
	vAssertTrue(xTransport.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "feedback ordering matrix primary route");
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "feedback ordering matrix primary failure");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED, "feedback ordering matrix primary failure decision");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "feedback ordering matrix primary budget one");
	vAssertTrue(pxSupervisorReport->eBudgetChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "feedback ordering matrix primary budget channel");
	vAssertTrue(pxSupervisorReport->eLastBudgetUpdate == RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED, "feedback ordering matrix primary failure budget update");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "feedback ordering matrix final established");
}

static void vTestSupervisorChannelEventOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x7AU };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "channel event ordering matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel event ordering matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "channel event ordering matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "channel event ordering matrix establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "channel event ordering matrix supervisor init");

	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel event ordering matrix failover");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel event ordering matrix selected secondary");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel event ordering matrix switch count after failover");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 1U, "channel event ordering matrix failover switch flag");
	vAssertTrue(pxSupervisorReport->uFailoverSwitchCount == 1U, "channel event ordering matrix failover count");
	vAssertTrue(pxSupervisorReport->uPreferredRecoverySwitchCount == 0U, "channel event ordering matrix preferred recovery count before recovery");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "channel event ordering matrix immediate preferred recovery count before recovery");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix holdoff preferred recovery count before recovery");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix completed holdoff preferred recovery count before recovery");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "channel event ordering matrix bypass preferred recovery count before recovery");
	vAssertTrue(pxSupervisorReport->uNoOpRefreshCount == 0U, "channel event ordering matrix no-op count before recovery");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 0U, "channel event ordering matrix preferred-triggered refresh count before recovery");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "channel event ordering matrix non-preferred-triggered refresh count before recovery");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 1U, "channel event ordering matrix preferred-triggered switch count after failover");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 0U, "channel event ordering matrix non-preferred-triggered switch count before recovery");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 0U, "channel event ordering matrix preferred-triggered no-op count before recovery");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "channel event ordering matrix non-preferred-triggered no-op count before recovery");
	vAssertTrue(pxSupervisorReport->uHoldoffRefreshNoOpCount == 0U, "channel event ordering matrix holdoff no-op count before recovery");
	vAssertTrue(pxSupervisorReport->uActiveRefreshNoOpCount == 0U, "channel event ordering matrix active no-op count before recovery");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 0U, "channel event ordering matrix holdoff cycle count before recovery");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "channel event ordering matrix completed holdoff cycle count before recovery");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "channel event ordering matrix aborted holdoff cycle count before recovery");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_NONE, "channel event ordering matrix holdoff cycle state before recovery");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "channel event ordering matrix completed holdoff cycle kind before recovery");
	vAssertTrue(pxSupervisorReport->eLastSwitchKind == RSRX_SUPERVISOR_SWITCH_KIND_FAILOVER, "channel event ordering matrix failover switch kind");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_FAILOVER_CHANNEL_DOWN, "channel event ordering matrix failover switch reason");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_DOWN, "channel event ordering matrix failover trigger event");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel event ordering matrix failover trigger channel");
	vAssertTrue(pxSupervisorReport->eLastSwitchFromChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel event ordering matrix failover switch from");
	vAssertTrue(pxSupervisorReport->eLastSwitchToChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel event ordering matrix failover switch to");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED, "channel event ordering matrix failover decision");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel event ordering matrix secondary refresh noop");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel event ordering matrix secondary retained on noop");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel event ordering matrix switch count unchanged on noop");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 0U, "channel event ordering matrix noop switch flag cleared");
	vAssertTrue(pxSupervisorReport->uFailoverSwitchCount == 1U, "channel event ordering matrix failover count retained on noop");
	vAssertTrue(pxSupervisorReport->uPreferredRecoverySwitchCount == 0U, "channel event ordering matrix preferred recovery count retained on noop");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "channel event ordering matrix immediate preferred recovery count retained on noop");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix holdoff preferred recovery count retained on noop");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix completed holdoff preferred recovery count retained on noop");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "channel event ordering matrix bypass preferred recovery count retained on noop");
	vAssertTrue(pxSupervisorReport->uNoOpRefreshCount == 1U, "channel event ordering matrix no-op count after first noop");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 0U, "channel event ordering matrix preferred-triggered refresh count after first noop");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 1U, "channel event ordering matrix non-preferred-triggered refresh count after first noop");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 1U, "channel event ordering matrix preferred-triggered switch count retained after first noop");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 0U, "channel event ordering matrix non-preferred-triggered switch count after first noop");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 0U, "channel event ordering matrix preferred-triggered no-op count after first noop");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 1U, "channel event ordering matrix non-preferred-triggered no-op count after first noop");
	vAssertTrue(pxSupervisorReport->uHoldoffRefreshNoOpCount == 1U, "channel event ordering matrix holdoff no-op count after first noop");
	vAssertTrue(pxSupervisorReport->uActiveRefreshNoOpCount == 0U, "channel event ordering matrix active no-op count after first noop");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 0U, "channel event ordering matrix holdoff cycle count after first noop");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "channel event ordering matrix completed holdoff cycle count after first noop");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "channel event ordering matrix aborted holdoff cycle count after first noop");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_NONE, "channel event ordering matrix holdoff cycle state after first noop");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "channel event ordering matrix completed holdoff cycle kind after first noop");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 0U, "channel event ordering matrix holdoff progress after first noop");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 0U, "channel event ordering matrix holdoff remaining after first noop");
	vAssertTrue(pxSupervisorReport->eLastSwitchKind == RSRX_SUPERVISOR_SWITCH_KIND_NONE, "channel event ordering matrix noop switch kind");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_REFRESH_NOOP, "channel event ordering matrix noop switch reason");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_UP, "channel event ordering matrix noop trigger event");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel event ordering matrix noop trigger channel");
	vAssertTrue(pxSupervisorReport->eLastSwitchFromChannelId == RSRX_TRANSPORT_CHANNEL_INVALID, "channel event ordering matrix noop switch from invalid");
	vAssertTrue(pxSupervisorReport->eLastSwitchToChannelId == RSRX_TRANSPORT_CHANNEL_INVALID, "channel event ordering matrix noop switch to invalid");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel event ordering matrix secondary refresh decision");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel event ordering matrix primary recovery refresh");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel event ordering matrix selected primary after recovery");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "channel event ordering matrix switch count after recovery");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 1U, "channel event ordering matrix recovery switch occurred");
	vAssertTrue(pxSupervisorReport->uFailoverSwitchCount == 1U, "channel event ordering matrix failover count after recovery");
	vAssertTrue(pxSupervisorReport->uPreferredRecoverySwitchCount == 1U, "channel event ordering matrix preferred recovery count after recovery");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 1U, "channel event ordering matrix immediate preferred recovery count after recovery");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix holdoff preferred recovery count after recovery");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix completed holdoff preferred recovery count after recovery");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "channel event ordering matrix bypass preferred recovery count after recovery");
	vAssertTrue(pxSupervisorReport->uNoOpRefreshCount == 1U, "channel event ordering matrix no-op count retained after recovery");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 1U, "channel event ordering matrix preferred-triggered refresh count after recovery");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 1U, "channel event ordering matrix non-preferred-triggered refresh count after recovery");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 2U, "channel event ordering matrix preferred-triggered switch count after recovery");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 0U, "channel event ordering matrix non-preferred-triggered switch count retained after recovery");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 0U, "channel event ordering matrix preferred-triggered no-op count retained after recovery");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 1U, "channel event ordering matrix non-preferred-triggered no-op count retained after recovery");
	vAssertTrue(pxSupervisorReport->uHoldoffRefreshNoOpCount == 1U, "channel event ordering matrix holdoff no-op count retained after recovery");
	vAssertTrue(pxSupervisorReport->uActiveRefreshNoOpCount == 0U, "channel event ordering matrix active no-op count retained after recovery");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 0U, "channel event ordering matrix holdoff cycle count retained after recovery");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "channel event ordering matrix completed holdoff cycle count retained after recovery");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "channel event ordering matrix aborted holdoff cycle count retained after recovery");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_NONE, "channel event ordering matrix holdoff cycle state after recovery");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "channel event ordering matrix completed holdoff cycle kind after recovery");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 0U, "channel event ordering matrix holdoff progress reset after recovery");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 0U, "channel event ordering matrix holdoff remaining after recovery");
	vAssertTrue(pxSupervisorReport->eLastSwitchKind == RSRX_SUPERVISOR_SWITCH_KIND_PREFERRED_RECOVERY, "channel event ordering matrix recovery switch kind");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_IMMEDIATE, "channel event ordering matrix recovery switch reason");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_UP, "channel event ordering matrix recovery trigger event");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel event ordering matrix recovery trigger channel");
	vAssertTrue(pxSupervisorReport->eLastSwitchFromChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel event ordering matrix recovery switch from");
	vAssertTrue(pxSupervisorReport->eLastSwitchToChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel event ordering matrix recovery switch to");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED, "channel event ordering matrix primary refresh decision");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel event ordering matrix repeated primary refresh noop");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel event ordering matrix primary retained on repeated refresh");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 2U, "channel event ordering matrix repeated refresh switch count unchanged");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 0U, "channel event ordering matrix repeated refresh switch flag cleared");
	vAssertTrue(pxSupervisorReport->uFailoverSwitchCount == 1U, "channel event ordering matrix failover count retained on repeated noop");
	vAssertTrue(pxSupervisorReport->uPreferredRecoverySwitchCount == 1U, "channel event ordering matrix preferred recovery count retained on repeated noop");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 1U, "channel event ordering matrix immediate preferred recovery count retained on repeated noop");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix holdoff preferred recovery count retained on repeated noop");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "channel event ordering matrix completed holdoff preferred recovery count retained on repeated noop");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "channel event ordering matrix bypass preferred recovery count retained on repeated noop");
	vAssertTrue(pxSupervisorReport->uNoOpRefreshCount == 2U, "channel event ordering matrix no-op count after repeated noop");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 2U, "channel event ordering matrix preferred-triggered refresh count after repeated noop");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 1U, "channel event ordering matrix non-preferred-triggered refresh count after repeated noop");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 2U, "channel event ordering matrix preferred-triggered switch count retained after repeated noop");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 0U, "channel event ordering matrix non-preferred-triggered switch count retained after repeated noop");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 1U, "channel event ordering matrix preferred-triggered no-op count after repeated noop");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 1U, "channel event ordering matrix non-preferred-triggered no-op count after repeated noop");
	vAssertTrue(pxSupervisorReport->uHoldoffRefreshNoOpCount == 1U, "channel event ordering matrix holdoff no-op count after repeated noop");
	vAssertTrue(pxSupervisorReport->uActiveRefreshNoOpCount == 1U, "channel event ordering matrix active no-op count after repeated noop");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 0U, "channel event ordering matrix holdoff cycle count after repeated noop");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "channel event ordering matrix completed holdoff cycle count after repeated noop");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "channel event ordering matrix aborted holdoff cycle count after repeated noop");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_NONE, "channel event ordering matrix holdoff cycle state after repeated noop");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "channel event ordering matrix completed holdoff cycle kind after repeated noop");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 0U, "channel event ordering matrix holdoff progress zero on active noop");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 0U, "channel event ordering matrix holdoff remaining on active noop");
	vAssertTrue(pxSupervisorReport->eLastSwitchKind == RSRX_SUPERVISOR_SWITCH_KIND_NONE, "channel event ordering matrix repeated noop switch kind");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_ACTIVE_REFRESH_NOOP, "channel event ordering matrix repeated noop switch reason");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_UP, "channel event ordering matrix repeated noop trigger event");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel event ordering matrix repeated noop trigger channel");
	vAssertTrue(pxSupervisorReport->eLastSwitchFromChannelId == RSRX_TRANSPORT_CHANNEL_INVALID, "channel event ordering matrix repeated noop switch from invalid");
	vAssertTrue(pxSupervisorReport->eLastSwitchToChannelId == RSRX_TRANSPORT_CHANNEL_INVALID, "channel event ordering matrix repeated noop switch to invalid");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel event ordering matrix final established");
}

static void vTestSupervisorSwitchAuditCloseoutMatrix(void)
{
	vTestSupervisorChannelEventOrderingMatrix();
}

static void vTestSupervisorSwitchAuditCumulativeMatrix(void)
{
	vTestSupervisorChannelEventOrderingMatrix();
}

static void vTestSupervisorSwitchAuditReasonMatrix(void)
{
	vTestSupervisorChannelEventOrderingMatrix();
}

static void vTestSupervisorSwitchAuditHoldoffProgressMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x6BU };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	xConfig.xChannelManagerConfig.uPreferredRecoveryHoldoffSelections = 2U;
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "switch audit holdoff progress matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "switch audit holdoff progress matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "switch audit holdoff progress matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "switch audit holdoff progress matrix establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "switch audit holdoff progress matrix supervisor init");

	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff progress matrix failover");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 0U, "switch audit holdoff progress matrix failover progress zero");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffTargetCount == 2U, "switch audit holdoff progress matrix failover target");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff progress matrix first hold");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_REFRESH_NOOP, "switch audit holdoff progress matrix first hold reason");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 1U, "switch audit holdoff progress matrix first hold cycle count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix first hold completed cycle count");
	vAssertTrue(pxSupervisorReport->uOrdinaryCompletedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix first hold ordinary-completed cycle count");
	vAssertTrue(pxSupervisorReport->uBypassCompletedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix first hold bypass-completed cycle count");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix first hold aborted cycle count");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_IN_PROGRESS, "switch audit holdoff progress matrix first hold state");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "switch audit holdoff progress matrix first hold completed kind");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 1U, "switch audit holdoff progress matrix first hold preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "switch audit holdoff progress matrix first hold non-preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 1U, "switch audit holdoff progress matrix first hold preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "switch audit holdoff progress matrix first hold non-preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix first hold immediate preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix first hold holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix first hold completed holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix first hold bypass preferred recovery count");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 1U, "switch audit holdoff progress matrix first hold progress");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffTargetCount == 2U, "switch audit holdoff progress matrix first hold target");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 1U, "switch audit holdoff progress matrix first hold remaining");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff progress matrix recovery");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_AFTER_HOLDOFF, "switch audit holdoff progress matrix recovery reason");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 1U, "switch audit holdoff progress matrix recovery cycle count retained");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 1U, "switch audit holdoff progress matrix recovery completed cycle count");
	vAssertTrue(pxSupervisorReport->uOrdinaryCompletedHoldoffCycleCount == 1U, "switch audit holdoff progress matrix recovery ordinary-completed cycle count");
	vAssertTrue(pxSupervisorReport->uBypassCompletedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix recovery bypass-completed cycle count");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix recovery aborted cycle count");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_COMPLETED, "switch audit holdoff progress matrix recovery state");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_ORDINARY, "switch audit holdoff progress matrix recovery completed kind");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 2U, "switch audit holdoff progress matrix recovery preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "switch audit holdoff progress matrix recovery non-preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 1U, "switch audit holdoff progress matrix recovery preferred-triggered no-op count retained");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "switch audit holdoff progress matrix recovery non-preferred-triggered no-op count retained");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix recovery immediate preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 1U, "switch audit holdoff progress matrix recovery holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 1U, "switch audit holdoff progress matrix recovery completed holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix recovery bypass preferred recovery count");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 0U, "switch audit holdoff progress matrix recovery progress reset");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffTargetCount == 2U, "switch audit holdoff progress matrix recovery target retained");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 2U, "switch audit holdoff progress matrix recovery remaining reset");

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff progress matrix active repeated refresh");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_ACTIVE_REFRESH_NOOP, "switch audit holdoff progress matrix active repeated refresh reason");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 1U, "switch audit holdoff progress matrix active repeated refresh cycle count retained");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 1U, "switch audit holdoff progress matrix active repeated refresh completed cycle count retained");
	vAssertTrue(pxSupervisorReport->uOrdinaryCompletedHoldoffCycleCount == 1U, "switch audit holdoff progress matrix active repeated refresh ordinary-completed cycle count retained");
	vAssertTrue(pxSupervisorReport->uBypassCompletedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix active repeated refresh bypass-completed cycle count retained");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "switch audit holdoff progress matrix active repeated refresh aborted cycle count retained");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_COMPLETED, "switch audit holdoff progress matrix active repeated refresh state");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_ORDINARY, "switch audit holdoff progress matrix active repeated refresh completed kind retained");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 3U, "switch audit holdoff progress matrix active repeated refresh preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "switch audit holdoff progress matrix active repeated refresh non-preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 2U, "switch audit holdoff progress matrix active repeated refresh preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "switch audit holdoff progress matrix active repeated refresh non-preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix active repeated refresh immediate preferred recovery count retained");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 1U, "switch audit holdoff progress matrix active repeated refresh holdoff preferred recovery count retained");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 1U, "switch audit holdoff progress matrix active repeated refresh completed holdoff preferred recovery count retained");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "switch audit holdoff progress matrix active repeated refresh bypass preferred recovery count retained");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 0U, "switch audit holdoff progress matrix active repeated refresh progress zero");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffTargetCount == 2U, "switch audit holdoff progress matrix active repeated refresh target retained");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 2U, "switch audit holdoff progress matrix active repeated refresh remaining retained");
}

static void vTestSupervisorSwitchAuditHoldoffResetMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x6CU };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	xConfig.xChannelManagerConfig.uPreferredRecoveryHoldoffSelections = 2U;
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "switch audit holdoff reset matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "switch audit holdoff reset matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "switch audit holdoff reset matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "switch audit holdoff reset matrix establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "switch audit holdoff reset matrix supervisor init");

	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff reset matrix failover");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff reset matrix first hold");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 1U, "switch audit holdoff reset matrix first hold progress");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 1U, "switch audit holdoff reset matrix first hold remaining");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 1U, "switch audit holdoff reset matrix first hold cycle count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix first hold completed cycle count");
	vAssertTrue(pxSupervisorReport->uOrdinaryCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix first hold ordinary-completed cycle count");
	vAssertTrue(pxSupervisorReport->uBypassCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix first hold bypass-completed cycle count");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix first hold aborted cycle count");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_IN_PROGRESS, "switch audit holdoff reset matrix first hold state");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "switch audit holdoff reset matrix first hold completed kind");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 1U, "switch audit holdoff reset matrix first hold preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "switch audit holdoff reset matrix first hold non-preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 1U, "switch audit holdoff reset matrix first hold preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "switch audit holdoff reset matrix first hold non-preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix first hold immediate preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix first hold holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix first hold completed holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix first hold bypass preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffResetCount == 0U, "switch audit holdoff reset matrix first hold reset count");

	// cppcheck-suppress redundantAssignment
	xTransport.uPrimaryAvailable = 0U;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff reset matrix reset");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 0U, "switch audit holdoff reset matrix no switch on reset");
	vAssertTrue(pxSupervisorReport->eLastSwitchKind == RSRX_SUPERVISOR_SWITCH_KIND_NONE, "switch audit holdoff reset matrix switch kind");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_RESET_CHANNEL_DOWN, "switch audit holdoff reset matrix reset reason");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 1U, "switch audit holdoff reset matrix reset cycle count retained");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix reset completed cycle count retained");
	vAssertTrue(pxSupervisorReport->uOrdinaryCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix reset ordinary-completed cycle count retained");
	vAssertTrue(pxSupervisorReport->uBypassCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix reset bypass-completed cycle count retained");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 1U, "switch audit holdoff reset matrix reset aborted cycle count");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_ABORTED, "switch audit holdoff reset matrix reset state");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "switch audit holdoff reset matrix reset completed kind");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 1U, "switch audit holdoff reset matrix reset preferred-triggered refresh count retained");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "switch audit holdoff reset matrix reset non-preferred-triggered refresh count retained");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 1U, "switch audit holdoff reset matrix reset preferred-triggered no-op count retained");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "switch audit holdoff reset matrix reset non-preferred-triggered no-op count retained");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix reset immediate preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix reset holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix reset completed holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix reset bypass preferred recovery count");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_DOWN, "switch audit holdoff reset matrix trigger event");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "switch audit holdoff reset matrix trigger channel");
	vAssertTrue(pxSupervisorReport->uHoldoffResetCount == 1U, "switch audit holdoff reset matrix reset count");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 0U, "switch audit holdoff reset matrix progress reset");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffTargetCount == 2U, "switch audit holdoff reset matrix target retained");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 2U, "switch audit holdoff reset matrix remaining reset");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit holdoff reset matrix hold after reset");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_REFRESH_NOOP, "switch audit holdoff reset matrix hold reason after reset");
	vAssertTrue(pxSupervisorReport->uHoldoffRefreshNoOpCount == 2U, "switch audit holdoff reset matrix hold count after reset");
	vAssertTrue(pxSupervisorReport->uHoldoffCycleCount == 2U, "switch audit holdoff reset matrix cycle count after reset");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix completed cycle count after reset");
	vAssertTrue(pxSupervisorReport->uOrdinaryCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix ordinary-completed cycle count after reset");
	vAssertTrue(pxSupervisorReport->uBypassCompletedHoldoffCycleCount == 0U, "switch audit holdoff reset matrix bypass-completed cycle count after reset");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 1U, "switch audit holdoff reset matrix aborted cycle count after reset");
	vAssertTrue(pxSupervisorReport->eLastHoldoffCycleState == RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_IN_PROGRESS, "switch audit holdoff reset matrix hold after reset state");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE, "switch audit holdoff reset matrix hold after reset completed kind");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredRefreshEventCount == 2U, "switch audit holdoff reset matrix hold after reset preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredRefreshEventCount == 0U, "switch audit holdoff reset matrix hold after reset non-preferred-triggered refresh count");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 1U, "switch audit holdoff reset matrix preferred-triggered switch count retained");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 0U, "switch audit holdoff reset matrix non-preferred-triggered switch count retained");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredNoOpRefreshCount == 2U, "switch audit holdoff reset matrix hold after reset preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredNoOpRefreshCount == 0U, "switch audit holdoff reset matrix hold after reset non-preferred-triggered no-op count");
	vAssertTrue(pxSupervisorReport->uImmediatePreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix hold after reset immediate preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix hold after reset holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix hold after reset completed holdoff preferred recovery count");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 0U, "switch audit holdoff reset matrix hold after reset bypass preferred recovery count");
	vAssertTrue(pxSupervisorReport->uHoldoffResetCount == 1U, "switch audit holdoff reset matrix reset count retained");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffProgressCount == 1U, "switch audit holdoff reset matrix progress after reset");
	vAssertTrue(pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount == 1U, "switch audit holdoff reset matrix remaining after reset");
}

static void vTestSupervisorSwitchAuditTriggerOriginMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[1] = { 0x71U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 1U;
	xTransport.uSecondaryAvailable = 1U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vSetActiveStandbyConfig(&xConfig);
	xConfig.xChannelManagerConfig.uPreferredRecoveryHoldoffSelections = 2U;
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "switch audit trigger origin matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "switch audit trigger origin matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "switch audit trigger origin matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "switch audit trigger origin matrix establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "switch audit trigger origin matrix supervisor init");

	xTransport.uPrimaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit trigger origin matrix failover");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 1U, "switch audit trigger origin matrix preferred-triggered switch count after failover");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 0U, "switch audit trigger origin matrix non-preferred-triggered switch count after failover");

	xTransport.uPrimaryAvailable = 1U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_UP;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit trigger origin matrix hold");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 1U, "switch audit trigger origin matrix preferred-triggered switch count retained on hold");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 0U, "switch audit trigger origin matrix non-preferred-triggered switch count retained on hold");

	xTransport.uSecondaryAvailable = 0U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "switch audit trigger origin matrix bypass");
	vAssertTrue(pxSupervisorReport->uPreferredChannelTriggeredSwitchCount == 1U, "switch audit trigger origin matrix preferred-triggered switch count retained on bypass");
	vAssertTrue(pxSupervisorReport->uNonPreferredChannelTriggeredSwitchCount == 1U, "switch audit trigger origin matrix non-preferred-triggered switch count after bypass");
	vAssertTrue(pxSupervisorReport->eLastSwitchTriggerChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "switch audit trigger origin matrix bypass trigger channel");
	vAssertTrue(pxSupervisorReport->eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_BYPASS_ACTIVE_LOSS, "switch audit trigger origin matrix bypass reason");
	vAssertTrue(pxSupervisorReport->uHoldoffPreferredRecoverySwitchCount == 1U, "switch audit trigger origin matrix holdoff preferred recovery count after bypass");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffPreferredRecoverySwitchCount == 0U, "switch audit trigger origin matrix completed holdoff preferred recovery count after bypass");
	vAssertTrue(pxSupervisorReport->uBypassPreferredRecoverySwitchCount == 1U, "switch audit trigger origin matrix bypass preferred recovery count after bypass");
	vAssertTrue(pxSupervisorReport->uCompletedHoldoffCycleCount == 1U, "switch audit trigger origin matrix completed cycle count after bypass");
	vAssertTrue(pxSupervisorReport->uOrdinaryCompletedHoldoffCycleCount == 0U, "switch audit trigger origin matrix ordinary-completed cycle count after bypass");
	vAssertTrue(pxSupervisorReport->uBypassCompletedHoldoffCycleCount == 1U, "switch audit trigger origin matrix bypass-completed cycle count after bypass");
	vAssertTrue(pxSupervisorReport->uAbortedHoldoffCycleCount == 0U, "switch audit trigger origin matrix aborted cycle count after bypass");
	vAssertTrue(pxSupervisorReport->eLastCompletedHoldoffCycleKind == RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_BYPASS, "switch audit trigger origin matrix completed kind after bypass");
}

static void vTestSupervisorSwitchAuditHoldoffOutcomeMatrix(void)
{
	vTestSupervisorSwitchAuditHoldoffProgressMatrix();
	vTestSupervisorSwitchAuditHoldoffResetMatrix();
	vTestSupervisorSwitchAuditTriggerOriginMatrix();
}

static void vTestSupervisorSwitchAuditEnvelopeMatrix(void)
{
	vTestSupervisorSwitchAuditCloseoutMatrix();
	vTestSupervisorSwitchAuditCumulativeMatrix();
	vTestSupervisorSwitchAuditReasonMatrix();
	vTestSupervisorSwitchAuditHoldoffOutcomeMatrix();
}

static void vTestSupervisorTimerDelegationMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x81U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "timer delegation matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix session connect");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix supervisor init");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_SUPERVISION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix connecting supervision");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "timer delegation matrix connecting timeout disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_TIMEOUT_EXPIRED, "timer delegation matrix connecting timeout reason");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "timer delegation matrix connecting timeout status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED, "timer delegation matrix connecting timeout decision");
	vAssertTrue(pxSupervisorReport->uAcceptedDecisionCount == 1U, "timer delegation matrix connecting accepted count");

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "timer delegation matrix established session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix established session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix established session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix established supervisor init");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_SUPERVISION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix established supervision");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "timer delegation matrix established timeout disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_TIMEOUT_EXPIRED, "timer delegation matrix established timeout reason");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "timer delegation matrix established timeout status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED, "timer delegation matrix established timeout decision");
	vAssertTrue(pxSupervisorReport->uAcceptedDecisionCount == 1U, "timer delegation matrix established accepted count");

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "timer delegation matrix retrans session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix retrans session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix retrans session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix retrans establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix retrans supervisor init");
	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		0U);
	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix retrans gap");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "timer delegation matrix retrans pending");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_RETRANSMISSION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix retrans timeout");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "timer delegation matrix retrans timeout disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RETRANSMISSION_FAILED, "timer delegation matrix retrans timeout reason");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "timer delegation matrix retrans timeout status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED, "timer delegation matrix retrans timeout decision");
	vAssertTrue(pxSupervisorReport->uAcceptedDecisionCount == 2U, "timer delegation matrix retrans accepted count");

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "timer delegation matrix invalid session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix invalid session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix invalid session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "timer delegation matrix invalid establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "timer delegation matrix invalid supervisor init");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_INVALID, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_SESSION_ERROR, "timer delegation matrix invalid source");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "timer delegation matrix invalid source leaves state");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_INVALID_ARGUMENT, "timer delegation matrix invalid source status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED, "timer delegation matrix invalid source decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_ACCEPTED, "timer delegation matrix invalid source decision class");
	vAssertTrue(pxSupervisorReport->uAcceptedDecisionCount == 1U, "timer delegation matrix invalid source accepted count");
	vAssertTrue(pxSupervisorReport->uErrorDecisionCount == 0U, "timer delegation matrix invalid source error count");
}

static void vTestSupervisorRecoverySuccessFromRetransmissionPending(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	rsrx_transport_frame_t xFrame;
	static const uint8_t auPayload[2] = { 0x81U, 0x82U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "recovery session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "recovery session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "recovery session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "recovery establish");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "recovery supervisor init");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		1U,
		0U);
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "recovery first inbound");

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		3U,
		0U);
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "recovery gap detection");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_RETRANSMISSION_PENDING, "recovery retransmission pending");

	vSetCodecBehavior(
		RSRX_CODEC_STATUS_OK,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_EVENT_VALID_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		2U,
		2U);
	vAssertTrue(rsrx_transport_supervisor_process_frame(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "recovery success frame");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "recovery returns to established");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_RECOVERY_COMPLETED, "recovery reason");
}

static void vTestSupervisorTimerExpiryDelegation(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x73U };

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "timer session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "timer session connect");
	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "timer supervisor init");

	vAssertTrue(rsrx_transport_supervisor_process_timer_expiry(&xSupervisor, RSRX_TIMER_EXPIRY_SUPERVISION, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "timer expiry status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "timer expiry safe disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_TIMEOUT_EXPIRED, "timer expiry reason");
	vAssertTrue(pxSupervisorReport->eLastSessionStatus == RSRX_STATUS_REJECTED, "timer expiry session status");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED, "timer expiry decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_ACCEPTED, "timer expiry class");
	vAssertTrue(pxSupervisorReport->uAcceptedDecisionCount == 1U, "timer expiry accepted count");
}

static void vTestSupervisorPumpReceiveBoundedDrain(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayloadA[1] = { 0x91U };
	static const uint8_t auPayloadB[1] = { 0x92U };
	rsrx_transport_frame_t axFrames[3];
	rsrx_transport_status_t aeStatuses[3];
	rsrx_codec_status_t aeCodecStatuses[2];
	rsrx_decoded_message_t axMessages[2];

	vInitTransportContext(&xTransport, auPayloadA, sizeof(auPayloadA), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayloadA, sizeof(auPayloadA));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump establish");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auPayloadA;
	axFrames[0].xPayloadLength = sizeof(auPayloadA);
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auPayloadB;
	axFrames[1].xPayloadLength = sizeof(auPayloadB);
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[2].puPayload = (const uint8_t *)0;
	axFrames[2].xPayloadLength = 0U;
	axFrames[2].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 3U);

	aeCodecStatuses[0] = RSRX_CODEC_STATUS_OK;
	aeCodecStatuses[1] = RSRX_CODEC_STATUS_OK;
	axMessages[0].eMessageType = RSRX_MESSAGE_TYPE_DATA;
	axMessages[0].eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	axMessages[0].eReason = RSRX_REASON_DATA_ACCEPTED;
	axMessages[0].uSequenceNumber = 1U;
	axMessages[0].uConfirmationNumber = 0U;
	axMessages[0].xPayloadLength = 0U;
	axMessages[1] = axMessages[0];
	axMessages[1].uSequenceNumber = 2U;
	vSetCodecScript(aeCodecStatuses, axMessages, 2U);

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump supervisor init");

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 4U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "pump receive status");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 3U, "pump iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 2U, "pump processed frames");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 2U, "pump cumulative processed frames");
	vAssertTrue(pxSupervisorReport->uPollCount == 3U, "pump poll count");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE, "pump terminal decision");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "pump state remains established");
}

static void vTestSupervisorPumpReceiveTerminalOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayload[1] = { 0x93U };
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_codec_status_t aeCodecStatuses[1];
	rsrx_decoded_message_t axMessages[1];

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump matrix supervisor init");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auPayload;
	axFrames[0].xPayloadLength = sizeof(auPayload);
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1] = axFrames[0];
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_NONE;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);
	aeCodecStatuses[0] = RSRX_CODEC_STATUS_OK;
	axMessages[0].eMessageType = RSRX_MESSAGE_TYPE_DATA;
	axMessages[0].eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	axMessages[0].eReason = RSRX_REASON_DATA_ACCEPTED;
	axMessages[0].uSequenceNumber = 1U;
	axMessages[0].uConfirmationNumber = 0U;
	axMessages[0].xPayloadLength = 0U;
	vSetCodecScript(aeCodecStatuses, axMessages, 1U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "pump matrix processed then idle");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 2U, "pump matrix processed then idle iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 1U, "pump matrix processed then idle processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE, "pump matrix processed then idle decision");

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.eReceiveStatus = RSRX_TRANSPORT_STATUS_UNAVAILABLE;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump matrix idle session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix idle session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix idle session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix idle establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump matrix idle supervisor init");

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_NO_FRAME, "pump matrix immediate idle");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 1U, "pump matrix immediate idle iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 0U, "pump matrix immediate idle processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE, "pump matrix immediate idle decision");

	vInitTransportContext(&xTransport, auPayload, sizeof(auPayload), RSRX_TRANSPORT_EVENT_NONE);
	xTransport.uPrimaryAvailable = 0U;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayload, sizeof(auPayload));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump matrix gated session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix gated session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump matrix gated session connect");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump matrix gated supervisor init");

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN, "pump matrix immediate gated");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 1U, "pump matrix immediate gated iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 0U, "pump matrix immediate gated processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN, "pump matrix immediate gated decision");
}

static void vTestSupervisorPumpReceiveErrorOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayloadA[1] = { 0x94U };
	static const uint8_t auPayloadB[1] = { 0x95U };
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_codec_status_t aeCodecStatuses[2];
	rsrx_decoded_message_t axMessages[2];

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;

	vInitTransportContext(&xTransport, auPayloadA, sizeof(auPayloadA), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayloadA, sizeof(auPayloadA));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump error matrix gated session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump error matrix gated session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump error matrix gated session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump error matrix gated establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump error matrix gated supervisor init");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auPayloadA;
	axFrames[0].xPayloadLength = sizeof(auPayloadA);
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1] = axFrames[0];
	axFrames[1].puPayload = auPayloadB;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_CHANNEL_DOWN;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);
	aeCodecStatuses[0] = RSRX_CODEC_STATUS_OK;
	axMessages[0].eMessageType = RSRX_MESSAGE_TYPE_DATA;
	axMessages[0].eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	axMessages[0].eReason = RSRX_REASON_DATA_ACCEPTED;
	axMessages[0].uSequenceNumber = 1U;
	axMessages[0].uConfirmationNumber = 0U;
	axMessages[0].xPayloadLength = 0U;
	vSetCodecScript(aeCodecStatuses, axMessages, 1U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN, "pump error matrix processed then gated");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 2U, "pump error matrix processed then gated iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 1U, "pump error matrix processed then gated processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN, "pump error matrix processed then gated decision");

	vInitTransportContext(&xTransport, auPayloadA, sizeof(auPayloadA), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayloadA, sizeof(auPayloadA));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump error matrix decode session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump error matrix decode session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump error matrix decode session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump error matrix decode establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump error matrix decode supervisor init");

	axFrames[0].puPayload = auPayloadA;
	axFrames[1].puPayload = auPayloadB;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);
	aeCodecStatuses[0] = RSRX_CODEC_STATUS_OK;
	aeCodecStatuses[1] = RSRX_CODEC_STATUS_DECODE_ERROR;
	axMessages[1] = axMessages[0];
	axMessages[1].uSequenceNumber = 2U;
	vSetCodecScript(aeCodecStatuses, axMessages, 2U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_DECODE_FAILED, "pump error matrix processed then decode fail");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 2U, "pump error matrix processed then decode fail iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 1U, "pump error matrix processed then decode fail processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_DECODE_FAILED, "pump error matrix processed then decode fail decision");
}

static void vTestSupervisorPumpReceiveIgnoredOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayloadA[1] = { 0x96U };
	static const uint8_t auPayloadB[1] = { 0x97U };
	rsrx_transport_frame_t axFrames[2];
	rsrx_transport_status_t aeStatuses[2];
	rsrx_codec_status_t aeCodecStatuses[1];
	rsrx_decoded_message_t axMessages[1];

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;

	vInitTransportContext(&xTransport, auPayloadA, sizeof(auPayloadA), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	xTransport.eReceiveStatus = RSRX_TRANSPORT_STATUS_RX_ERROR;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayloadA, sizeof(auPayloadA));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump ignored matrix immediate session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump ignored matrix immediate session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump ignored matrix immediate session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump ignored matrix immediate establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump ignored matrix immediate supervisor init");

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "pump ignored matrix immediate budgeted");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 1U, "pump ignored matrix immediate iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 0U, "pump ignored matrix immediate processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "pump ignored matrix immediate decision");

	vInitTransportContext(&xTransport, auPayloadA, sizeof(auPayloadA), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayloadA, sizeof(auPayloadA));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump ignored matrix processed session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump ignored matrix processed session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump ignored matrix processed session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump ignored matrix processed establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump ignored matrix processed supervisor init");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auPayloadA;
	axFrames[0].xPayloadLength = sizeof(auPayloadA);
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auPayloadB;
	axFrames[1].xPayloadLength = sizeof(auPayloadB);
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_RX_ERROR;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 2U);
	aeCodecStatuses[0] = RSRX_CODEC_STATUS_OK;
	axMessages[0].eMessageType = RSRX_MESSAGE_TYPE_DATA;
	axMessages[0].eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	axMessages[0].eReason = RSRX_REASON_DATA_ACCEPTED;
	axMessages[0].uSequenceNumber = 1U;
	axMessages[0].uConfirmationNumber = 0U;
	axMessages[0].xPayloadLength = 0U;
	vSetCodecScript(aeCodecStatuses, axMessages, 1U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 3U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "pump ignored matrix processed then budgeted");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 2U, "pump ignored matrix processed then budgeted iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 1U, "pump ignored matrix processed then budgeted processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED, "pump ignored matrix processed then budgeted decision");
}

static void vTestSupervisorPumpReceiveEscalationOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayloadA[1] = { 0x98U };

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;

	vInitTransportContext(&xTransport, auPayloadA, sizeof(auPayloadA), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	xTransport.eReceiveStatus = RSRX_TRANSPORT_STATUS_RX_ERROR;
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayloadA, sizeof(auPayloadA));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump escalation matrix immediate session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump escalation matrix immediate session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump escalation matrix immediate session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump escalation matrix immediate establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump escalation matrix immediate supervisor init");

	vAssertTrue(rsrx_transport_supervisor_poll_receive(&xSupervisor, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "pump escalation matrix prime budget");
	vAssertTrue(pxSupervisorReport->uConsecutiveReceiveErrorCount == 1U, "pump escalation matrix prime count");

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "pump escalation matrix immediate status");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 1U, "pump escalation matrix immediate iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 0U, "pump escalation matrix immediate processed");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "pump escalation matrix immediate safe disconnect");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED, "pump escalation matrix immediate decision");
	vAssertTrue(pxSupervisorReport->eLastEffectiveEvent == RSRX_EVENT_PROTOCOL_ERROR, "pump escalation matrix immediate event");
}

static void vTestSupervisorPumpReceiveMaxPollOrderingMatrix(void)
{
	rsrx_session_t xSession;
	rsrx_session_config_t xConfig;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_codec_port_t xCodec;
	const rsrx_orchestrator_report_t * pxSessionReport;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	test_transport_context_t xTransport = { 0 };
	test_clock_context_t xClock = { 1000U };
	test_timer_context_t xTimer = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnostics = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_callback_context_t xCallbacks = { 0U, 0U, 0U };
	static const uint8_t auPayloadA[1] = { 0x9aU };
	static const uint8_t auPayloadB[1] = { 0x9bU };
	rsrx_transport_frame_t axFrames[3];
	rsrx_transport_status_t aeStatuses[3];
	rsrx_codec_status_t aeCodecStatuses[3];
	rsrx_decoded_message_t axMessages[3];

	xCodec.pfEncode = (rsrx_encode_message_fn)0;
	xCodec.pfDecode = eDecodeFrame;

	vInitTransportContext(&xTransport, auPayloadA, sizeof(auPayloadA), RSRX_TRANSPORT_EVENT_FRAME_RECEIVED);
	vFillConfig(&xConfig, &xTransport, &xClock, &xTimer, &xDiagnostics, &xCallbacks, auPayloadA, sizeof(auPayloadA));
	vAssertTrue(rsrx_session_init(&xSession, &xConfig) == RSRX_STATUS_OK, "pump max matrix session init");
	vAssertTrue(rsrx_session_start(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump max matrix session start");
	vAssertTrue(rsrx_session_connect(&xSession, &pxSessionReport) == RSRX_STATUS_OK, "pump max matrix session connect");
	vAssertTrue(rsrx_session_process_event(&xSession, RSRX_EVENT_HANDSHAKE_SUCCESS, &pxSessionReport) == RSRX_STATUS_OK, "pump max matrix establish");
	vAssertTrue(rsrx_transport_supervisor_init(&xSupervisor, &xSession, &xCodec) == RSRX_SUPERVISOR_STATUS_OK, "pump max matrix supervisor init");

	axFrames[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[0].puPayload = auPayloadA;
	axFrames[0].xPayloadLength = sizeof(auPayloadA);
	axFrames[0].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[1].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axFrames[1].puPayload = auPayloadB;
	axFrames[1].xPayloadLength = sizeof(auPayloadB);
	axFrames[1].eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	axFrames[2] = axFrames[1];
	aeStatuses[0] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[1] = RSRX_TRANSPORT_STATUS_OK;
	aeStatuses[2] = RSRX_TRANSPORT_STATUS_OK;
	vSetReceiveScript(&xTransport, axFrames, aeStatuses, 3U);

	aeCodecStatuses[0] = RSRX_CODEC_STATUS_OK;
	aeCodecStatuses[1] = RSRX_CODEC_STATUS_OK;
	aeCodecStatuses[2] = RSRX_CODEC_STATUS_OK;
	axMessages[0].eMessageType = RSRX_MESSAGE_TYPE_DATA;
	axMessages[0].eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	axMessages[0].eReason = RSRX_REASON_DATA_ACCEPTED;
	axMessages[0].uSequenceNumber = 1U;
	axMessages[0].uConfirmationNumber = 0U;
	axMessages[0].xPayloadLength = 0U;
	axMessages[1] = axMessages[0];
	axMessages[1].uSequenceNumber = 2U;
	axMessages[2] = axMessages[1];
	axMessages[2].uSequenceNumber = 3U;
	vSetCodecScript(aeCodecStatuses, axMessages, 3U);

	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 2U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "pump max matrix status");
	vAssertTrue(pxSupervisorReport->uLastPumpIterationCount == 2U, "pump max matrix iterations");
	vAssertTrue(pxSupervisorReport->uLastPumpProcessedFrameCount == 2U, "pump max matrix processed");
	vAssertTrue(pxSupervisorReport->uProcessedFrameCount == 2U, "pump max matrix cumulative processed");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED, "pump max matrix last decision");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "pump max matrix established");
}

static void vTestSupervisorPumpReceiveInvalidArguments(void)
{
	rsrx_transport_supervisor_context_t xSupervisor = { 0 };
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;

	vAssertTrue(rsrx_transport_supervisor_pump_receive((rsrx_transport_supervisor_context_t *)0, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "pump null context");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 0U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "pump zero max polls");
}

static void vTestSupervisorRuntimeOrderingCloseoutMatrix(void)
{
	vTestSupervisorBudgetScopeMatrix();
	vTestSupervisorSendFeedbackOrderingMatrix();
	vTestSupervisorChannelEventOrderingMatrix();
	vTestSupervisorSwitchAuditCloseoutMatrix();
	vTestSupervisorSwitchAuditCumulativeMatrix();
	vTestSupervisorSwitchAuditEnvelopeMatrix();
	vTestSupervisorTimerDelegationMatrix();
	vTestSupervisorPollReceiveRetryOrderingMatrix();
	vTestSupervisorPumpReceiveTerminalOrderingMatrix();
	vTestSupervisorPumpReceiveErrorOrderingMatrix();
	vTestSupervisorPumpReceiveIgnoredOrderingMatrix();
	vTestSupervisorPumpReceiveEscalationOrderingMatrix();
	vTestSupervisorPumpReceiveMaxPollOrderingMatrix();
}

int main(void)
{
	vTestSupervisorInboundHandshakePath();
	vTestSupervisorInvalidArguments();
	vTestSupervisorDecodeFailure();
	vTestSupervisorUnsupportedMessage();
	vTestSupervisorSequenceGapDetection();
	vTestSupervisorStaleSequenceProtocolError();
	vTestSupervisorPollReceiveHandshake();
	vTestSupervisorPollReceiveChannelDown();
	vTestSupervisorPollReceiveNoFrame();
	vTestSupervisorPollReceiveErrorBudgeted();
	vTestSupervisorPollReceiveErrorEscalatesAndResets();
	vTestSupervisorChannelDownUsesFailover();
	vTestSupervisorChannelUpRefreshesSelection();
	vTestSupervisorTransportSendFailed();
	vTestSupervisorTransportSendCompletedIgnored();
	vTestSupervisorTransportSendCompletedCorrelated();
	vTestSupervisorReportExposesBusyRejectTelemetry();
	vTestSupervisorSendFailureBudgetResetsAfterSuccess();
	vTestSupervisorTimerExpiryDelegation();
	vTestSupervisorRecoverySuccessFromRetransmissionPending();
	vTestSupervisorPumpReceiveBoundedDrain();
	vTestSupervisorRuntimeOrderingCloseoutMatrix();
	vTestSupervisorPumpReceiveInvalidArguments();

	(void)printf("rsrx_transport_supervisor_test: all tests passed\n");

	return EXIT_SUCCESS;
}
