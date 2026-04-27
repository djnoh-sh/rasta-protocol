#include <stdio.h>
#include <stdlib.h>

#include "rsrx_channel_manager.h"
#include "rsrx_platform_adapters.h"
#include "rsrx_codec.h"

typedef struct
{
	rsrx_monotonic_time_ns_t uNowNs;
	uint32_t uCallCount;
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
	rsrx_transport_send_request_t xLastRequest;
	uint32_t uCallCount;
	uint32_t uPrimaryAvailable;
	uint32_t uSecondaryAvailable;
	uint32_t uForceMismatchedQueryId;
} test_transport_context_t;

typedef struct
{
	rsrx_action_t eLastAction;
	uint32_t uCallCount;
} test_action_context_t;

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
	pxContext->uCallCount++;
	*puNowNs = pxContext->uNowNs;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t eTimerCommand(void * pvContext, const rsrx_timer_command_t * pxCommand)
{
	test_timer_context_t * pxContext = (test_timer_context_t *)pvContext;
	pxContext->uCallCount++;
	pxContext->xLastCommand = *pxCommand;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t eDiagnosticWrite(void * pvContext, const rsrx_diagnostic_record_t * pxRecord)
{
	test_diagnostics_context_t * pxContext = (test_diagnostics_context_t *)pvContext;
	pxContext->uCallCount++;
	pxContext->xLastRecord = *pxRecord;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_transport_status_t eTransportSend(void * pvContext, const rsrx_transport_send_request_t * pxRequest)
{
	test_transport_context_t * pxContext = (test_transport_context_t *)pvContext;
	pxContext->uCallCount++;
	pxContext->xLastRequest = *pxRequest;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t eTransportReceive(void * pvContext, rsrx_transport_frame_t * pxFrame)
{
	(void)pvContext;
	(void)pxFrame;
	return RSRX_TRANSPORT_STATUS_OK;
}

/* cppcheck-suppress constParameterCallback */
static rsrx_transport_status_t eTransportQuery(void * pvContext, rsrx_transport_channel_state_t * pxState)
{
	const test_transport_context_t * pxContext = (const test_transport_context_t *)pvContext;
	if(pxState != (rsrx_transport_channel_state_t *)0)
	{
		if(pxContext->uForceMismatchedQueryId != 0U)
		{
			pxState->eChannelId = (pxState->eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY) ?
				RSRX_TRANSPORT_CHANNEL_SECONDARY :
				RSRX_TRANSPORT_CHANNEL_PRIMARY;
			pxState->uIsAvailable = 1U;
			return RSRX_TRANSPORT_STATUS_OK;
		}
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
	return RSRX_TRANSPORT_STATUS_OK;
}

static void vCaptureAction(void * pvContext, const rsrx_transition_result_t * pxTransition, rsrx_action_t eAction, uint32_t uActionIndex)
{
	test_action_context_t * pxContext = (test_action_context_t *)pvContext;
	(void)pxTransition;
	(void)uActionIndex;
	pxContext->uCallCount++;
	pxContext->eLastAction = eAction;
}

static void vInitSingleChannelManager(
	rsrx_channel_manager_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId)
{
	rsrx_channel_manager_config_t xConfig;

	xConfig.eMode = RSRX_REDUNDANCY_MODE_SINGLE;
	xConfig.uChannelCount = 1U;
	xConfig.uPreferredChannelIndex = 0U;
	xConfig.uPreferredRecoveryHoldoffSelections = 0U;
	xConfig.uPreferredRecoveryFlapPenaltySelections = 0U;
	xConfig.axChannels[0].eChannelId = eChannelId;
	xConfig.axChannels[0].uIsAvailable = 1U;
	xConfig.axChannels[0].uPriority = 0U;
	xConfig.axChannels[1].eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	xConfig.axChannels[1].uIsAvailable = 0U;
	xConfig.axChannels[1].uPriority = 0U;
	vAssertTrue(
		rsrx_channel_manager_init(pxContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"channel manager init");
}

static void vInitActiveStandbyChannelManager(
	rsrx_channel_manager_context_t * pxContext)
{
	rsrx_channel_manager_config_t xConfig;

	xConfig.eMode = RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY;
	xConfig.uChannelCount = 2U;
	xConfig.uPreferredChannelIndex = 0U;
	xConfig.uPreferredRecoveryHoldoffSelections = 0U;
	xConfig.uPreferredRecoveryFlapPenaltySelections = 0U;
	xConfig.axChannels[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xConfig.axChannels[0].uIsAvailable = 1U;
	xConfig.axChannels[0].uPriority = 0U;
	xConfig.axChannels[1].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xConfig.axChannels[1].uIsAvailable = 1U;
	xConfig.axChannels[1].uPriority = 1U;
	vAssertTrue(
		rsrx_channel_manager_init(pxContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active standby channel manager init");
}

static void vInitActiveStandbyHoldoffChannelManager(
	rsrx_channel_manager_context_t * pxContext,
	uint32_t uHoldoffSelections)
{
	rsrx_channel_manager_config_t xConfig;

	xConfig.eMode = RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY;
	xConfig.uChannelCount = 2U;
	xConfig.uPreferredChannelIndex = 0U;
	xConfig.uPreferredRecoveryHoldoffSelections = uHoldoffSelections;
	xConfig.uPreferredRecoveryFlapPenaltySelections = 0U;
	xConfig.axChannels[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xConfig.axChannels[0].uIsAvailable = 1U;
	xConfig.axChannels[0].uPriority = 0U;
	xConfig.axChannels[1].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xConfig.axChannels[1].uIsAvailable = 1U;
	xConfig.axChannels[1].uPriority = 1U;
	vAssertTrue(
		rsrx_channel_manager_init(pxContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active standby holdoff channel manager init");
}

static void vTestPlatformExecutorTableBuild(void)
{
	rsrx_platform_adapter_context_t xPlatformContext;
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	test_clock_context_t xClockContext = { 100U, 0U };
	test_timer_context_t xTimerContext = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnosticsContext = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 1U, 0U };
	test_action_context_t xApplicationContext = { RSRX_ACTION_NONE, 0U };
	test_action_context_t xApiContext = { RSRX_ACTION_NONE, 0U };
	test_action_context_t xLifecycleContext = { RSRX_ACTION_NONE, 0U };
	rsrx_platform_port_table_t xPorts;
	rsrx_transport_port_t xTransportPort;
	rsrx_action_executor_t xApplicationExecutor;
	rsrx_action_executor_t xApiExecutor;
	rsrx_action_executor_t xLifecycleExecutor;
	rsrx_action_executor_table_t xExecutors;
	rsrx_status_t eStatus;
	static const uint8_t auPayload[4] = { 0x10U, 0x20U, 0x30U, 0x40U };

	xPorts.xClock.pvContext = &xClockContext;
	xPorts.xClock.pfNow = eClockNow;
	xPorts.xTimer.pvContext = &xTimerContext;
	xPorts.xTimer.pfCommand = eTimerCommand;
	xPorts.xDiagnostics.pvContext = &xDiagnosticsContext;
	xPorts.xDiagnostics.pfWrite = eDiagnosticWrite;

	vAssertTrue(rsrx_platform_adapter_init(&xPlatformContext, &xPorts, 50U, 75U, 125U) == RSRX_PLATFORM_STATUS_OK, "platform adapter init");
	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);
	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auPayload,
			sizeof(auPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"transport adapter init");

	xApplicationExecutor.pvContext = &xApplicationContext;
	xApplicationExecutor.pfDispatch = vCaptureAction;
	xApiExecutor.pvContext = &xApiContext;
	xApiExecutor.pfDispatch = vCaptureAction;
	xLifecycleExecutor.pvContext = &xLifecycleContext;
	xLifecycleExecutor.pfDispatch = vCaptureAction;

	eStatus = rsrx_platform_adapter_build_executor_table(
		&xExecutors,
		&xTransportAdapterContext,
		&xPlatformContext,
		&xApplicationExecutor,
		&xApiExecutor,
		&xLifecycleExecutor);
	vAssertTrue(eStatus == RSRX_STATUS_OK, "build executor table");
	vAssertTrue(xExecutors.xTransportExecutor.pfDispatch == rsrx_transport_executor_dispatch, "transport executor binding");
	vAssertTrue(xExecutors.xTimerExecutor.pfDispatch == rsrx_platform_timer_executor_dispatch, "timer executor binding");
	vAssertTrue(xExecutors.xApplicationExecutor.pfDispatch == vCaptureAction, "application executor binding");
	vAssertTrue(xExecutors.xDiagnosticsExecutor.pfDispatch == rsrx_platform_diagnostics_executor_dispatch, "diagnostics executor binding");
}

static void vTestTransportTimerAndDiagnosticsDispatch(void)
{
	rsrx_platform_adapter_context_t xPlatformContext;
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	test_clock_context_t xClockContext = { 1000U, 0U };
	test_timer_context_t xTimerContext = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnosticsContext = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 1U, 0U };
	rsrx_platform_port_table_t xPorts;
	rsrx_transport_port_t xTransportPort;
	rsrx_transition_result_t xTransition;
	static const uint8_t auPayload[2] = { 0xAAU, 0x55U };

	xPorts.xClock.pvContext = &xClockContext;
	xPorts.xClock.pfNow = eClockNow;
	xPorts.xTimer.pvContext = &xTimerContext;
	xPorts.xTimer.pfCommand = eTimerCommand;
	xPorts.xDiagnostics.pvContext = &xDiagnosticsContext;
	xPorts.xDiagnostics.pfWrite = eDiagnosticWrite;

	(void)rsrx_platform_adapter_init(&xPlatformContext, &xPorts, 200U, 300U, 400U);
	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);
	(void)rsrx_transport_adapter_init(
		&xTransportAdapterContext,
		&xTransportPort,
		rsrx_codec_get_default_port(),
		&xChannelManagerContext,
		RSRX_TRANSPORT_CHANNEL_PRIMARY,
		auPayload,
		sizeof(auPayload));

	xTransition.ePreviousState = RSRX_STATE_INITIALIZED;
	xTransition.eNextState = RSRX_STATE_CONNECTING;
	xTransition.eStatus = RSRX_STATUS_OK;
	xTransition.eReason = RSRX_REASON_CONNECT_REQUESTED;
	xTransition.eDiagnostic = RSRX_DIAG_INFO_STATE_TRANSITION;
	xTransition.xActions.uActionCount = 0U;

	rsrx_transport_executor_dispatch(&xTransportAdapterContext, &xTransition, RSRX_ACTION_START_HANDSHAKE, 0U);
	vAssertTrue(xTransportContext.uCallCount == 1U, "transport send called");
	vAssertTrue(xTransportContext.xLastRequest.xPayloadLength == D_RSRX_CODEC_HEADER_BYTES, "handshake payload encoded");
	vAssertTrue(xTransportContext.xLastRequest.puPayload[0] == (uint8_t)RSRX_MESSAGE_TYPE_CONNECT_REQUEST, "handshake message type encoded");
	vAssertTrue(xTransportContext.xLastRequest.puPayload[1] == (uint8_t)RSRX_REASON_CONNECT_REQUESTED, "handshake reason encoded");

	xTransition.ePreviousState = RSRX_STATE_ESTABLISHED;
	xTransition.eNextState = RSRX_STATE_ESTABLISHED;
	xTransition.eReason = RSRX_REASON_DATA_ACCEPTED;
	rsrx_transport_adapter_record_inbound_message(
		&xTransportAdapterContext,
		&(rsrx_decoded_message_t){
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_EVENT_VALID_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			7U,
			2U,
			{ 0xABU, 0xCDU },
			2U });
	{
		const rsrx_decoded_message_t * pxLastMessage =
			rsrx_transport_adapter_get_last_inbound_message(&xTransportAdapterContext);
		vAssertTrue(pxLastMessage != (const rsrx_decoded_message_t *)0, "last inbound message available");
		vAssertTrue(pxLastMessage->eMessageType == RSRX_MESSAGE_TYPE_DATA, "last inbound type");
		vAssertTrue(pxLastMessage->eReason == RSRX_REASON_DATA_ACCEPTED, "last inbound reason");
		vAssertTrue(pxLastMessage->uSequenceNumber == 7U, "last inbound sequence");
		vAssertTrue(pxLastMessage->uConfirmationNumber == 2U, "last inbound confirmation");
		vAssertTrue(pxLastMessage->xPayloadLength == 2U, "last inbound payload length");
		vAssertTrue(pxLastMessage->auPayload[0] == 0xABU, "last inbound payload copied");
	}

	rsrx_platform_timer_executor_dispatch(&xPlatformContext, &xTransition, RSRX_ACTION_START_SUPERVISION_TIMER, 1U);
	vAssertTrue(xClockContext.uCallCount == 1U, "clock called");
	vAssertTrue(xTimerContext.uCallCount == 1U, "timer command called");
	vAssertTrue(xTimerContext.xLastCommand.eTimerId == RSRX_TIMER_ID_SUPERVISION, "timer id mapped");
	vAssertTrue(xTimerContext.xLastCommand.eCommandType == RSRX_TIMER_COMMAND_START, "timer command mapped");
	vAssertTrue(xTimerContext.xLastCommand.uDeadlineNs == 1200U, "timer deadline computed");

	rsrx_platform_diagnostics_executor_dispatch(&xPlatformContext, &xTransition, RSRX_ACTION_LOG_DIAGNOSTIC, 3U);
	vAssertTrue(xDiagnosticsContext.uCallCount == 1U, "diagnostics write called");
	vAssertTrue(xDiagnosticsContext.xLastRecord.eReason == RSRX_REASON_DATA_ACCEPTED, "diagnostic reason propagated");
	vAssertTrue(xDiagnosticsContext.xLastRecord.eSeverity == RSRX_LOG_SEVERITY_INFO, "diagnostic severity mapped");
	vAssertTrue(xDiagnosticsContext.xLastRecord.uEventCounter == 1U, "diagnostic event counter incremented");
}

static void vTestApplicationDataSend(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 1U, 0U };
	rsrx_transport_port_t xTransportPort;
	static const uint8_t auFramePayload[2] = { 0xAAU, 0x55U };
	static const uint8_t auDataPayload[3] = { 0x31U, 0x32U, 0x33U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"transport adapter init for application send");
	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(&xTransportAdapterContext);
	vAssertTrue(pxTelemetry != (const rsrx_outbound_send_telemetry_t *)0, "application data telemetry available");

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data send");
	vAssertTrue(xTransportContext.uCallCount == 1U, "application data send count");
	vAssertTrue(xTransportContext.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "application data channel");
	vAssertTrue(xTransportContext.xLastRequest.eReason == RSRX_REASON_APPLICATION_DATA_REQUESTED, "application data reason");
	vAssertTrue(xTransportContext.xLastRequest.xPayloadLength == (D_RSRX_CODEC_HEADER_BYTES + sizeof(auDataPayload)), "application data encoded length");
	vAssertTrue(xTransportContext.xLastRequest.puPayload[0] == (uint8_t)RSRX_MESSAGE_TYPE_DATA, "application data message type");
	vAssertTrue(xTransportContext.xLastRequest.puPayload[1] == (uint8_t)RSRX_REASON_APPLICATION_DATA_REQUESTED, "application data reason encoded");
	vAssertTrue(xTransportContext.xLastRequest.puPayload[7] == 0x01U, "application data sequence encoded");
	vAssertTrue(xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auDataPayload[0], "application data payload copied");
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xTransportAdapterContext) == 1U, "application data outstanding send set");
	vAssertTrue(pxTelemetry->uAcceptedSendCount == 1U, "application data accepted telemetry");
	vAssertTrue(pxTelemetry->eLastSendStatus == RSRX_TRANSPORT_STATUS_OK, "application data last send telemetry");
	vAssertTrue(pxTelemetry->eLastRejectReason == RSRX_OUTBOUND_REJECT_REASON_NONE, "application data no reject reason after success");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data second send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 1U, "application data queued telemetry");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 0U, "application data busy telemetry before overflow");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data third send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 2U, "application data queued telemetry second slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data fourth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 3U, "application data queued telemetry third slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data fifth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 4U, "application data queued telemetry fourth slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data sixth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 5U, "application data queued telemetry fifth slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data seventh send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 6U, "application data queued telemetry sixth slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data eighth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 7U, "application data queued telemetry seventh slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data ninth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 8U, "application data queued telemetry eighth slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data tenth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 9U, "application data queued telemetry ninth slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data eleventh send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 10U, "application data queued telemetry tenth slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data twelfth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 11U, "application data queued telemetry eleventh slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data thirteenth send queued");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 12U, "application data queued telemetry twelfth slot");
	vAssertTrue(pxTelemetry->uMaxDeferredSendCount == 12U, "application data max deferred telemetry twelfth slot");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"application data fourteenth send overflow");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 1U, "application data busy telemetry one");
	vAssertTrue(pxTelemetry->uQueueOverflowRejectCount == 1U, "application data queue overflow telemetry");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "application data busy streak one");
	vAssertTrue(pxTelemetry->uMaxConsecutiveBusyRejectedSendCount == 1U, "application data busy max one");
	vAssertTrue(pxTelemetry->eLastSendStatus == RSRX_TRANSPORT_STATUS_UNAVAILABLE, "application data busy status telemetry");
	vAssertTrue(pxTelemetry->eLastRejectReason == RSRX_OUTBOUND_REJECT_REASON_QUEUE_OVERFLOW, "application data overflow reject reason");

	xTransportAdapterContext.xLastInboundMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xTransportAdapterContext.xLastInboundMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xTransportAdapterContext.xLastInboundMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xTransportAdapterContext.xLastInboundMessage.uSequenceNumber = 2U;
	xTransportAdapterContext.xLastInboundMessage.uConfirmationNumber = 1U;
	xTransportAdapterContext.xLastInboundMessage.xPayloadLength = sizeof(auDataPayload);
	rsrx_transport_adapter_record_inbound_message(
		&xTransportAdapterContext,
		&xTransportAdapterContext.xLastInboundMessage);
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xTransportAdapterContext) == 1U, "application data deferred dispatched on inbound");
	vAssertTrue(pxTelemetry->uClearOnInboundCount == 1U, "application data inbound clear telemetry");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 1U, "application data deferred dispatch telemetry");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 0U, "application data busy streak reset by inbound");
	vAssertTrue(xTransportContext.uCallCount == 2U, "application data deferred send count");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data send queued after dispatch");
	vAssertTrue(pxTelemetry->uQueuedSendCount == 13U, "application data queued telemetry after dispatch");
	vAssertTrue(pxTelemetry->eLastRejectReason == RSRX_OUTBOUND_REJECT_REASON_NONE, "application data queued clears reject reason");
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(xTransportContext.uCallCount == 3U, "application data send count after clear");
	vAssertTrue(pxTelemetry->uAcceptedSendCount == 3U, "application data accepted telemetry after clear");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 2U, "application data deferred dispatch telemetry after clear");
	vAssertTrue(pxTelemetry->uMaxConsecutiveBusyRejectedSendCount == 1U, "application data busy max retained");
}

static void vTestChannelManagerDrivenFailoverSelection(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 0U, 1U, 0U };
	rsrx_transport_port_t xTransportPort;
	rsrx_transition_result_t xTransition;
	rsrx_transport_channel_state_t xChannelState;
	static const uint8_t auFramePayload[2] = { 0xAAU, 0x55U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitActiveStandbyChannelManager(&xChannelManagerContext);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"transport adapter init for failover");

	vAssertTrue(
		rsrx_transport_adapter_query_channel(
			&xTransportAdapterContext,
			&xChannelState) == RSRX_TRANSPORT_STATUS_OK,
		"query failover channel");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "selected secondary channel");

	xTransition.ePreviousState = RSRX_STATE_INITIALIZED;
	xTransition.eNextState = RSRX_STATE_CONNECTING;
	xTransition.eStatus = RSRX_STATUS_OK;
	xTransition.eReason = RSRX_REASON_CONNECT_REQUESTED;
	xTransition.eDiagnostic = RSRX_DIAG_INFO_STATE_TRANSITION;
	xTransition.xActions.uActionCount = 0U;

	rsrx_transport_executor_dispatch(&xTransportAdapterContext, &xTransition, RSRX_ACTION_START_HANDSHAKE, 0U);
	vAssertTrue(xTransportContext.uCallCount == 1U, "failover send called");
	vAssertTrue(xTransportContext.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "failover send channel");
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);

	xTransportContext.uPrimaryAvailable = 1U;
	xTransportContext.uSecondaryAvailable = 1U;
	vAssertTrue(
		rsrx_transport_adapter_query_channel(
			&xTransportAdapterContext,
			&xChannelState) == RSRX_TRANSPORT_STATUS_OK,
		"query preferred recovery channel");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "recovered preferred channel");

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"preferred recovery send called");
	vAssertTrue(xTransportContext.uCallCount == 2U, "preferred recovery send count");
	vAssertTrue(xTransportContext.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "preferred recovery send channel");
}

static void vTestChannelManagerQueryRejectsTopologyMutation(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 1U, 1U };
	rsrx_transport_port_t xTransportPort;
	rsrx_transport_channel_state_t xChannelState;
	static const uint8_t auFramePayload[2] = { 0xAAU, 0x55U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitActiveStandbyChannelManager(&xChannelManagerContext);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"transport adapter init for query mutation reject");

	vAssertTrue(
		rsrx_transport_adapter_query_channel(
			&xTransportAdapterContext,
			&xChannelState) == RSRX_TRANSPORT_STATUS_RX_ERROR,
		"query mutation rejected as rx error");
	vAssertTrue(
		rsrx_channel_manager_get_active_channel(&xChannelManagerContext) == RSRX_TRANSPORT_CHANNEL_PRIMARY,
		"query mutation active channel retained");
	vAssertTrue(xTransportContext.uCallCount == 0U, "query mutation no send");
}

static void vTestPreferredRecoveryHoldoffSelection(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 0U, 1U, 0U };
	rsrx_transport_port_t xTransportPort;
	rsrx_transition_result_t xTransition;
	rsrx_transport_channel_state_t xChannelState;
	static const uint8_t auFramePayload[2] = { 0xAAU, 0x55U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitActiveStandbyHoldoffChannelManager(&xChannelManagerContext, 2U);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"transport adapter init for holdoff");

	vAssertTrue(
		rsrx_transport_adapter_query_channel(
			&xTransportAdapterContext,
			&xChannelState) == RSRX_TRANSPORT_STATUS_OK,
		"holdoff failover query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff failover secondary");

	xTransition.ePreviousState = RSRX_STATE_INITIALIZED;
	xTransition.eNextState = RSRX_STATE_CONNECTING;
	xTransition.eStatus = RSRX_STATUS_OK;
	xTransition.eReason = RSRX_REASON_CONNECT_REQUESTED;
	xTransition.eDiagnostic = RSRX_DIAG_INFO_STATE_TRANSITION;
	xTransition.xActions.uActionCount = 0U;

	xTransportContext.uPrimaryAvailable = 1U;
	xTransportContext.uSecondaryAvailable = 1U;
	vAssertTrue(
		rsrx_transport_adapter_query_channel(
			&xTransportAdapterContext,
			&xChannelState) == RSRX_TRANSPORT_STATUS_OK,
		"holdoff first recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff retains secondary");
	vAssertTrue(rsrx_channel_manager_get_active_channel(&xChannelManagerContext) == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff active remains secondary");
	rsrx_transport_executor_dispatch(&xTransportAdapterContext, &xTransition, RSRX_ACTION_START_HANDSHAKE, 0U);
	vAssertTrue(xTransportContext.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff first recovery send secondary");
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);

	vAssertTrue(
		rsrx_transport_adapter_query_channel(
			&xTransportAdapterContext,
			&xChannelState) == RSRX_TRANSPORT_STATUS_OK,
		"holdoff second recovery query");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff switches to primary");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"holdoff second recovery send");
	vAssertTrue(xTransportContext.xLastRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff second recovery send primary");
}

static void vTestBusyRejectEscalationTelemetry(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 0U, 0U };
	rsrx_transport_port_t xTransportPort;
	static const uint8_t auFramePayload[2] = { 0x55U, 0xAAU };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject escalation telemetry init");
	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(&xTransportAdapterContext);
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 0U, "busy reject escalation count initial");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 0U, "busy reject escalation latch initial");

	rsrx_transport_adapter_note_busy_reject_escalation(&xTransportAdapterContext);
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 1U, "busy reject escalation count increment");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 1U, "busy reject escalation latch set");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 0U, "busy reject escalation latch reset on clear");
}

static void vTestApplicationDataDeferredQueueFifoDispatch(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 0U, 0U };
	rsrx_transport_port_t xTransportPort;
	static const uint8_t auFramePayload[2] = { 0x91U, 0x92U };
	static const uint8_t auFirstPayload[2] = { 0x41U, 0x42U };
	static const uint8_t auSecondPayload[2] = { 0x51U, 0x52U };
	static const uint8_t auThirdPayload[2] = { 0x61U, 0x62U };
	static const uint8_t auFourthPayload[2] = { 0x71U, 0x72U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data fifo init");
	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(&xTransportAdapterContext);

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFirstPayload,
			sizeof(auFirstPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data fifo first send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSecondPayload,
			sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data fifo second queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auThirdPayload,
			sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data fifo third queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFourthPayload,
			sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data fifo fourth queued");

	xTransportAdapterContext.xLastInboundMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xTransportAdapterContext.xLastInboundMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xTransportAdapterContext.xLastInboundMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xTransportAdapterContext.xLastInboundMessage.uSequenceNumber = 2U;
	xTransportAdapterContext.xLastInboundMessage.uConfirmationNumber = 1U;
	xTransportAdapterContext.xLastInboundMessage.xPayloadLength = sizeof(auFramePayload);
	rsrx_transport_adapter_record_inbound_message(
		&xTransportAdapterContext,
		&xTransportAdapterContext.xLastInboundMessage);
	vAssertTrue(xTransportContext.uCallCount == 2U, "application data fifo first dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auSecondPayload[0],
		"application data fifo first dispatch payload");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 1U, "application data fifo first dispatch telemetry");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(xTransportContext.uCallCount == 3U, "application data fifo second dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auThirdPayload[0],
		"application data fifo second dispatch payload");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 2U, "application data fifo second dispatch telemetry");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(xTransportContext.uCallCount == 4U, "application data fifo third dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auFourthPayload[0],
		"application data fifo third dispatch payload");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 3U, "application data fifo third dispatch telemetry");
}

static void vTestApplicationDataDeferredQueueMixedClearLongRun(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 0U, 0U };
	rsrx_transport_port_t xTransportPort;
	static const uint8_t auFramePayload[2] = { 0xA1U, 0xA2U };
	static const uint8_t auFirstPayload[2] = { 0x11U, 0x12U };
	static const uint8_t auSecondPayload[2] = { 0x21U, 0x22U };
	static const uint8_t auThirdPayload[2] = { 0x31U, 0x32U };
	static const uint8_t auFourthPayload[2] = { 0x41U, 0x42U };
	static const uint8_t auFifthPayload[2] = { 0x51U, 0x52U };
	static const uint8_t auSixthPayload[2] = { 0x61U, 0x62U };
	static const uint8_t auSeventhPayload[2] = { 0x71U, 0x72U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run init");
	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(&xTransportAdapterContext);

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFirstPayload,
			sizeof(auFirstPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run first send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSecondPayload,
			sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run second queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auThirdPayload,
			sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run third queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFourthPayload,
			sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run fourth queued");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(xTransportContext.uCallCount == 2U, "application data mixed clear long run first feedback dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auSecondPayload[0],
		"application data mixed clear long run first feedback dispatch payload");

	xTransportAdapterContext.xLastInboundMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xTransportAdapterContext.xLastInboundMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xTransportAdapterContext.xLastInboundMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xTransportAdapterContext.xLastInboundMessage.uSequenceNumber = 2U;
	xTransportAdapterContext.xLastInboundMessage.uConfirmationNumber = 1U;
	xTransportAdapterContext.xLastInboundMessage.xPayloadLength = sizeof(auFramePayload);
	rsrx_transport_adapter_record_inbound_message(
		&xTransportAdapterContext,
		&xTransportAdapterContext.xLastInboundMessage);
	vAssertTrue(xTransportContext.uCallCount == 3U, "application data mixed clear long run first inbound dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auThirdPayload[0],
		"application data mixed clear long run first inbound dispatch payload");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(xTransportContext.uCallCount == 4U, "application data mixed clear long run trailing manual dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auFourthPayload[0],
		"application data mixed clear long run trailing manual dispatch payload");

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFifthPayload,
			sizeof(auFifthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run fifth send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSixthPayload,
			sizeof(auSixthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run sixth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSeventhPayload,
			sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data mixed clear long run seventh queued");

	xTransportAdapterContext.xLastInboundMessage.uSequenceNumber = 3U;
	xTransportAdapterContext.xLastInboundMessage.uConfirmationNumber = 1U;
	rsrx_transport_adapter_record_inbound_message(
		&xTransportAdapterContext,
		&xTransportAdapterContext.xLastInboundMessage);
	vAssertTrue(xTransportContext.uCallCount == 5U, "application data mixed clear long run second inbound dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auFifthPayload[0],
		"application data mixed clear long run second inbound dispatch payload");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(xTransportContext.uCallCount == 6U, "application data mixed clear long run second feedback dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auSixthPayload[0],
		"application data mixed clear long run second feedback dispatch payload");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(xTransportContext.uCallCount == 7U, "application data mixed clear long run trailing second feedback dispatch count");
	vAssertTrue(
		xTransportContext.xLastRequest.puPayload[D_RSRX_CODEC_HEADER_BYTES] == auSeventhPayload[0],
		"application data mixed clear long run trailing second feedback dispatch payload");

	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 6U, "application data mixed clear long run dispatch telemetry");
	vAssertTrue(pxTelemetry->uClearManualCount == 4U, "application data mixed clear long run manual clear telemetry");
	vAssertTrue(pxTelemetry->uClearOnInboundCount == 2U, "application data mixed clear long run inbound clear telemetry");
	vAssertTrue(pxTelemetry->uMaxDeferredSendCount == 3U, "application data mixed clear long run max deferred retained");
}

static void vTestBusyRejectThresholdManualInboundResetSources(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 0U, 0U };
	rsrx_transport_port_t xTransportPort;
	static const uint8_t auFramePayload[2] = { 0x71U, 0x72U };
	static const uint8_t auFirstPayload[2] = { 0x11U, 0x12U };
	static const uint8_t auSecondPayload[2] = { 0x21U, 0x22U };
	static const uint8_t auThirdPayload[2] = { 0x31U, 0x32U };
	static const uint8_t auFourthPayload[2] = { 0x41U, 0x42U };
	static const uint8_t auFifthPayload[2] = { 0x51U, 0x52U };
	static const uint8_t auSixthPayload[2] = { 0x61U, 0x62U };
	static const uint8_t auSeventhPayload[2] = { 0x71U, 0x72U };
	static const uint8_t auEighthPayload[2] = { 0x81U, 0x82U };
	static const uint8_t auNinthPayload[2] = { 0x91U, 0x92U };
	static const uint8_t auTenthPayload[2] = { 0xA1U, 0xA2U };
	static const uint8_t auEleventhPayload[2] = { 0xB1U, 0xB2U };
	static const uint8_t auTwelfthPayload[2] = { 0xC1U, 0xC2U };
	static const uint8_t auThirteenthPayload[2] = { 0xD1U, 0xD2U };
	static const uint8_t auFourteenthPayload[2] = { 0xE1U, 0xE2U };
	static const uint8_t auFifteenthPayload[2] = { 0xF1U, 0xF2U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset init");
	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(&xTransportAdapterContext);

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFirstPayload,
			sizeof(auFirstPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset first send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSecondPayload,
			sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset second queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auThirdPayload,
			sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset third queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFourthPayload,
			sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset fourth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFifthPayload,
			sizeof(auFifthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset fifth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSixthPayload,
			sizeof(auSixthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset sixth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSeventhPayload,
			sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset seventh queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auEighthPayload,
			sizeof(auEighthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset eighth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auNinthPayload,
			sizeof(auNinthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset ninth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auTenthPayload,
			sizeof(auTenthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset tenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auEleventhPayload,
			sizeof(auEleventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset eleventh queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auTwelfthPayload,
			sizeof(auTwelfthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset twelfth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auThirteenthPayload,
			sizeof(auThirteenthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset thirteenth queued");
	rsrx_transport_adapter_note_busy_reject_escalation(&xTransportAdapterContext);
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFourteenthPayload,
			sizeof(auFourteenthPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"busy reject manual inbound reset first reject");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFifteenthPayload,
			sizeof(auFifteenthPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"busy reject manual inbound reset second reject");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 2U, "busy reject manual inbound reset first streak");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 1U, "busy reject manual inbound reset first escalation");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 0U, "busy reject manual inbound reset manual streak reset");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 0U, "busy reject manual inbound reset manual latch reset");
	vAssertTrue(pxTelemetry->uClearManualCount == 1U, "busy reject manual inbound reset manual clear telemetry");

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSixthPayload,
			sizeof(auSixthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset post-manual queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSeventhPayload,
			sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"busy reject manual inbound reset post-manual reject");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "busy reject manual inbound reset post-manual streak restart");

	xTransportAdapterContext.xLastInboundMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xTransportAdapterContext.xLastInboundMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xTransportAdapterContext.xLastInboundMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xTransportAdapterContext.xLastInboundMessage.uSequenceNumber = 2U;
	xTransportAdapterContext.xLastInboundMessage.uConfirmationNumber = 1U;
	xTransportAdapterContext.xLastInboundMessage.xPayloadLength = sizeof(auFramePayload);
	rsrx_transport_adapter_record_inbound_message(
		&xTransportAdapterContext,
		&xTransportAdapterContext.xLastInboundMessage);
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 0U, "busy reject manual inbound reset inbound streak reset");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 0U, "busy reject manual inbound reset inbound latch reset");
	vAssertTrue(pxTelemetry->uClearOnInboundCount == 1U, "busy reject manual inbound reset inbound clear telemetry");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 2U, "busy reject manual inbound reset dispatch telemetry");

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auSeventhPayload,
			sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"busy reject manual inbound reset post-inbound queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auFourthPayload,
			sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"busy reject manual inbound reset post-inbound reject");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "busy reject manual inbound reset post-inbound streak restart");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 1U, "busy reject manual inbound reset escalation retained");
}

static void vTestDeferredQueueTelemetryAccumulationMatrix(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 0U, 0U };
	rsrx_transport_port_t xTransportPort;
	static const uint8_t auFramePayload[2] = { 0x91U, 0x92U };
	static const uint8_t auFirstPayload[2] = { 0x11U, 0x12U };
	static const uint8_t auSecondPayload[2] = { 0x21U, 0x22U };
	static const uint8_t auThirdPayload[2] = { 0x31U, 0x32U };
	static const uint8_t auFourthPayload[2] = { 0x41U, 0x42U };
	static const uint8_t auFifthPayload[2] = { 0x51U, 0x52U };
	static const uint8_t auSixthPayload[2] = { 0x61U, 0x62U };
	static const uint8_t auSeventhPayload[2] = { 0x71U, 0x72U };
	static const uint8_t auEighthPayload[2] = { 0x81U, 0x82U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation init");
	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(&xTransportAdapterContext);

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFirstPayload, sizeof(auFirstPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation first send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSecondPayload, sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation second queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auThirdPayload, sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation third queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFourthPayload, sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation fourth queued");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFifthPayload, sizeof(auFifthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation fifth send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSixthPayload, sizeof(auSixthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation sixth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSeventhPayload, sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation seventh queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auEighthPayload, sizeof(auEighthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"deferred queue telemetry accumulation eighth queued");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);

	vAssertTrue(pxTelemetry->uQueuedSendCount == 6U, "deferred queue telemetry accumulation queued count");
	vAssertTrue(pxTelemetry->uDeferredDispatchCount == 6U, "deferred queue telemetry accumulation dispatch count");
	vAssertTrue(pxTelemetry->uAcceptedSendCount == 8U, "deferred queue telemetry accumulation accepted count");
	vAssertTrue(pxTelemetry->uMaxDeferredSendCount == 3U, "deferred queue telemetry accumulation max deferred count");
	vAssertTrue(pxTelemetry->uClearManualCount == 8U, "deferred queue telemetry accumulation manual clear count");
}

static void vTestOverflowBusyAccumulationMatrix(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	const rsrx_outbound_send_telemetry_t * pxTelemetry;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 0U, 0U };
	rsrx_transport_port_t xTransportPort;
	static const uint8_t auFramePayload[2] = { 0xA1U, 0xA2U };
	static const uint8_t auFirstPayload[2] = { 0x11U, 0x12U };
	static const uint8_t auSecondPayload[2] = { 0x21U, 0x22U };
	static const uint8_t auThirdPayload[2] = { 0x31U, 0x32U };
	static const uint8_t auFourthPayload[2] = { 0x41U, 0x42U };
	static const uint8_t auFifthPayload[2] = { 0x51U, 0x52U };
	static const uint8_t auSixthPayload[2] = { 0x61U, 0x62U };
	static const uint8_t auSeventhPayload[2] = { 0x71U, 0x72U };
	static const uint8_t auEighthPayload[2] = { 0x81U, 0x82U };
	static const uint8_t auNinthPayload[2] = { 0x91U, 0x92U };

	xTransportPort.pvContext = &xTransportContext;
	xTransportPort.pfSend = eTransportSend;
	xTransportPort.pfReceive = eTransportReceive;
	xTransportPort.pfQueryChannel = eTransportQuery;
	vInitSingleChannelManager(&xChannelManagerContext, RSRX_TRANSPORT_CHANNEL_PRIMARY);

	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
			&xChannelManagerContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			auFramePayload,
			sizeof(auFramePayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation init");
	pxTelemetry = rsrx_transport_adapter_get_outbound_telemetry(&xTransportAdapterContext);

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFirstPayload, sizeof(auFirstPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation first send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSecondPayload, sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation second queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auThirdPayload, sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation third queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFourthPayload, sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation fourth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFifthPayload, sizeof(auFifthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation fifth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSixthPayload, sizeof(auSixthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation sixth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSeventhPayload, sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation seventh queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auEighthPayload, sizeof(auEighthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation eighth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auNinthPayload, sizeof(auNinthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation ninth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSecondPayload, sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation tenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auThirdPayload, sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation eleventh queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFourthPayload, sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation twelfth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFifthPayload, sizeof(auFifthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation thirteenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSecondPayload, sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"overflow busy accumulation first reject");
	vAssertTrue(pxTelemetry->uQueueOverflowRejectCount == 1U, "overflow busy accumulation overflow count one");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 1U, "overflow busy accumulation busy count one");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 1U, "overflow busy accumulation streak one");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 0U, "overflow busy accumulation streak reset");

	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auThirdPayload, sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation fifth send");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFourthPayload, sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation sixth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFifthPayload, sizeof(auFifthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation seventh queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSixthPayload, sizeof(auSixthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation eighth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSeventhPayload, sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation ninth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auEighthPayload, sizeof(auEighthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation tenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auNinthPayload, sizeof(auNinthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation eleventh queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSecondPayload, sizeof(auSecondPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation twelfth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auThirdPayload, sizeof(auThirdPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation thirteenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFifthPayload, sizeof(auFifthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation fourteenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSixthPayload, sizeof(auSixthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation fifteenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auSeventhPayload, sizeof(auSeventhPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation sixteenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auEighthPayload, sizeof(auEighthPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"overflow busy accumulation seventeenth queued");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFourthPayload, sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"overflow busy accumulation second reject");
	rsrx_transport_adapter_note_busy_reject_escalation(&xTransportAdapterContext);
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(&xTransportAdapterContext, auFourthPayload, sizeof(auFourthPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"overflow busy accumulation third reject");

	vAssertTrue(pxTelemetry->uQueueOverflowRejectCount == 3U, "overflow busy accumulation overflow count three");
	vAssertTrue(pxTelemetry->uBusyRejectedSendCount == 3U, "overflow busy accumulation busy count three");
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 2U, "overflow busy accumulation second streak");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 1U, "overflow busy accumulation escalation count");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 1U, "overflow busy accumulation escalation latch");
	vAssertTrue(pxTelemetry->uMaxConsecutiveBusyRejectedSendCount == 2U, "overflow busy accumulation max streak");
	vAssertTrue(pxTelemetry->uMaxDeferredSendCount == 12U, "overflow busy accumulation max deferred retained");

	rsrx_transport_adapter_clear_outstanding_send(&xTransportAdapterContext);
	vAssertTrue(pxTelemetry->uConsecutiveBusyRejectedSendCount == 0U, "overflow busy accumulation final streak reset");
	vAssertTrue(pxTelemetry->uLastBusyRejectEscalated == 0U, "overflow busy accumulation final latch reset");
	vAssertTrue(pxTelemetry->uQueueOverflowRejectCount == 3U, "overflow busy accumulation overflow retained");
	vAssertTrue(pxTelemetry->uBusyRejectEscalationCount == 1U, "overflow busy accumulation escalation retained");
	vAssertTrue(pxTelemetry->uMaxDeferredSendCount == 12U, "overflow busy accumulation final max deferred retained");
}

static void vTestOutboundQueueLongRunRepresentativeMatrix(void)
{
	vTestApplicationDataDeferredQueueMixedClearLongRun();
	vTestBusyRejectThresholdManualInboundResetSources();
	vTestDeferredQueueTelemetryAccumulationMatrix();
	vTestOverflowBusyAccumulationMatrix();
}

static void vTestOutboundQueueFairnessRepresentativeMatrix(void)
{
	vTestApplicationDataDeferredQueueFifoDispatch();
	vTestApplicationDataDeferredQueueMixedClearLongRun();
}

static void vTestOutboundQueueBackpressureCloseoutMatrix(void)
{
	vTestOutboundQueueFairnessRepresentativeMatrix();
	vTestOutboundQueueLongRunRepresentativeMatrix();
}

int main(void)
{
	vTestPlatformExecutorTableBuild();
	vTestTransportTimerAndDiagnosticsDispatch();
	vTestApplicationDataSend();
	vTestApplicationDataDeferredQueueFifoDispatch();
	vTestApplicationDataDeferredQueueMixedClearLongRun();
	vTestBusyRejectThresholdManualInboundResetSources();
	vTestDeferredQueueTelemetryAccumulationMatrix();
	vTestOverflowBusyAccumulationMatrix();
	vTestOutboundQueueBackpressureCloseoutMatrix();
	vTestChannelManagerDrivenFailoverSelection();
	vTestChannelManagerQueryRejectsTopologyMutation();
	vTestPreferredRecoveryHoldoffSelection();
	vTestBusyRejectEscalationTelemetry();

	(void)printf("rsrx_platform_adapters_test: all tests passed\n");

	return EXIT_SUCCESS;
}
