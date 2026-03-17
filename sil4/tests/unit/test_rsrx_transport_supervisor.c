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

	xTransport.uPrimaryAvailable = 0U;
	xTransport.uSecondaryAvailable = 1U;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "channel failover ignored");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "channel failover state retained");
	vAssertTrue(pxSupervisorReport->xLastChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "channel failover selected secondary");
	vAssertTrue(pxSupervisorReport->xLastChannelState.uIsAvailable == 1U, "channel failover selected available");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED, "channel failover decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "channel failover class");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 1U, "channel failover ignored count");
	vAssertTrue(pxSupervisorReport->uChannelSwitchCount == 1U, "channel failover switch count");
	vAssertTrue(pxSupervisorReport->uLastChannelSwitchOccurred == 1U, "channel failover switch occurred");
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
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;

	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "send failed first status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "send failed first keeps state");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 1U, "send failed first budget");
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED, "send failed first decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "send failed first class");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 1U, "send failed first ignored count");
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_OK, "send failed second status");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT, "send failed safe disconnect");
	vAssertTrue(pxSupervisorReport->pxLastReport->xTransition.eReason == RSRX_REASON_PROTOCOL_ERROR_DETECTED, "send failed reason");
	vAssertTrue(pxSupervisorReport->uConsecutiveSendFailureCount == 0U, "send failed budget reset");
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
	vAssertTrue(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED, "send complete decision");
	vAssertTrue(pxSupervisorReport->eLastDecisionClass == RSRX_SUPERVISOR_DECISION_CLASS_IGNORED, "send complete class");
	vAssertTrue(pxSupervisorReport->uIgnoredDecisionCount == 1U, "send complete ignored count");
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

	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_FAILED;
	vAssertTrue(rsrx_transport_supervisor_process_transport_event(&xSupervisor, &xFrame, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT, "budget reset failure after success");
	vAssertTrue(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED, "budget reset state remains established");
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

static void vTestSupervisorPumpReceiveInvalidArguments(void)
{
	rsrx_transport_supervisor_context_t xSupervisor = { 0 };
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;

	vAssertTrue(rsrx_transport_supervisor_pump_receive((rsrx_transport_supervisor_context_t *)0, 1U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "pump null context");
	vAssertTrue(rsrx_transport_supervisor_pump_receive(&xSupervisor, 0U, &pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT, "pump zero max polls");
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
	vTestSupervisorChannelDownUsesFailover();
	vTestSupervisorTransportSendFailed();
	vTestSupervisorTransportSendCompletedIgnored();
	vTestSupervisorSendFailureBudgetResetsAfterSuccess();
	vTestSupervisorTimerExpiryDelegation();
	vTestSupervisorRecoverySuccessFromRetransmissionPending();
	vTestSupervisorPumpReceiveBoundedDrain();
	vTestSupervisorPumpReceiveInvalidArguments();

	(void)printf("rsrx_transport_supervisor_test: all tests passed\n");

	return EXIT_SUCCESS;
}
