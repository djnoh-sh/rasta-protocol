#include <stdio.h>
#include <stdlib.h>

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

static void vCaptureAction(void * pvContext, const rsrx_transition_result_t * pxTransition, rsrx_action_t eAction, uint32_t uActionIndex)
{
	test_action_context_t * pxContext = (test_action_context_t *)pvContext;
	(void)pxTransition;
	(void)uActionIndex;
	pxContext->uCallCount++;
	pxContext->eLastAction = eAction;
}

static void vTestPlatformExecutorTableBuild(void)
{
	rsrx_platform_adapter_context_t xPlatformContext;
	rsrx_transport_adapter_context_t xTransportAdapterContext;
	test_clock_context_t xClockContext = { 100U, 0U };
	test_timer_context_t xTimerContext = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnosticsContext = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
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
	vAssertTrue(
		rsrx_transport_adapter_init(
			&xTransportAdapterContext,
			&xTransportPort,
			rsrx_codec_get_default_port(),
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
	test_clock_context_t xClockContext = { 1000U, 0U };
	test_timer_context_t xTimerContext = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnosticsContext = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_transport_context_t xTransportContext = { { RSRX_TRANSPORT_CHANNEL_INVALID, (const uint8_t *)0, 0U, RSRX_REASON_NONE }, 0U };
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
	(void)rsrx_transport_adapter_init(
		&xTransportAdapterContext,
		&xTransportPort,
		rsrx_codec_get_default_port(),
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

int main(void)
{
	vTestPlatformExecutorTableBuild();
	vTestTransportTimerAndDiagnosticsDispatch();

	(void)printf("rsrx_platform_adapters_test: all tests passed\n");

	return EXIT_SUCCESS;
}
