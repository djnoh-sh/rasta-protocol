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
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 1U };
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
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 1U };
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
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 1U, 1U };
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
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_UNAVAILABLE,
		"application data second send busy");

	xTransportAdapterContext.xLastInboundMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xTransportAdapterContext.xLastInboundMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xTransportAdapterContext.xLastInboundMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xTransportAdapterContext.xLastInboundMessage.uSequenceNumber = 2U;
	xTransportAdapterContext.xLastInboundMessage.uConfirmationNumber = 1U;
	xTransportAdapterContext.xLastInboundMessage.xPayloadLength = sizeof(auDataPayload);
	rsrx_transport_adapter_record_inbound_message(
		&xTransportAdapterContext,
		&xTransportAdapterContext.xLastInboundMessage);
	vAssertTrue(rsrx_transport_adapter_has_outstanding_send(&xTransportAdapterContext) == 0U, "application data outstanding cleared by inbound");
	vAssertTrue(
		rsrx_transport_adapter_send_application_data(
			&xTransportAdapterContext,
			auDataPayload,
			sizeof(auDataPayload)) == RSRX_TRANSPORT_STATUS_OK,
		"application data send after inbound clear");
	vAssertTrue(xTransportContext.uCallCount == 2U, "application data send count after clear");
}

static void vTestChannelManagerDrivenFailoverSelection(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 0U, 1U };
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

static void vTestPreferredRecoveryHoldoffSelection(void)
{
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	rsrx_channel_manager_context_t xChannelManagerContext;
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U, 0U, 1U };
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

int main(void)
{
	vTestPlatformExecutorTableBuild();
	vTestTransportTimerAndDiagnosticsDispatch();
	vTestApplicationDataSend();
	vTestChannelManagerDrivenFailoverSelection();
	vTestPreferredRecoveryHoldoffSelection();

	(void)printf("rsrx_platform_adapters_test: all tests passed\n");

	return EXIT_SUCCESS;
}
