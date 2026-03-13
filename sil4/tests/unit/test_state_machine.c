#include <stdio.h>
#include <stdlib.h>

#include "sil4_state_machine.h"

static void vAssertEqualState(
	sil4_state_t eExpected,
	sil4_state_t eActual,
	const char * pcMessage)
{
	if(eExpected != eActual)
	{
		(void)fprintf(
			stderr,
			"ASSERT FAILED: %s (expected=%d actual=%d)\n",
			pcMessage,
			(int)eExpected,
			(int)eActual);
		exit(EXIT_FAILURE);
	}
}

static void vAssertEqualStatus(
	sil4_status_t eExpected,
	sil4_status_t eActual,
	const char * pcMessage)
{
	if(eExpected != eActual)
	{
		(void)fprintf(
			stderr,
			"ASSERT FAILED: %s (expected=%d actual=%d)\n",
			pcMessage,
			(int)eExpected,
			(int)eActual);
		exit(EXIT_FAILURE);
	}
}

static void vAssertActionCount(
	uint32_t uExpected,
	uint32_t uActual,
	const char * pcMessage)
{
	if(uExpected != uActual)
	{
		(void)fprintf(
			stderr,
			"ASSERT FAILED: %s (expected=%u actual=%u)\n",
			pcMessage,
			(unsigned int)uExpected,
			(unsigned int)uActual);
		exit(EXIT_FAILURE);
	}
}

static void vAssertActionAt(
	sil4_action_t eExpected,
	const sil4_transition_result_t * pxResult,
	uint32_t uIndex,
	const char * pcMessage)
{
	if(pxResult->xActions.eActions[uIndex] != eExpected)
	{
		(void)fprintf(
			stderr,
			"ASSERT FAILED: %s (expected=%d actual=%d index=%u)\n",
			pcMessage,
			(int)eExpected,
			(int)pxResult->xActions.eActions[uIndex],
			(unsigned int)uIndex);
		exit(EXIT_FAILURE);
	}
}

static void vMoveToInitialized(
	sil4_state_machine_context_t * pxContext,
	sil4_transition_result_t * pxResult)
{
	(void)state_machine_init(pxContext);
	(void)state_machine_handle_event(pxContext, SIL4_EVENT_INIT_SUCCESS, pxResult);
}

static void vMoveToConnecting(
	sil4_state_machine_context_t * pxContext,
	sil4_transition_result_t * pxResult)
{
	vMoveToInitialized(pxContext, pxResult);
	(void)state_machine_handle_event(pxContext, SIL4_EVENT_CONNECT_REQUEST, pxResult);
}

static void vMoveToEstablished(
	sil4_state_machine_context_t * pxContext,
	sil4_transition_result_t * pxResult)
{
	vMoveToConnecting(pxContext, pxResult);
	(void)state_machine_handle_event(pxContext, SIL4_EVENT_HANDSHAKE_SUCCESS, pxResult);
}

static void vMoveToRetransmissionPending(
	sil4_state_machine_context_t * pxContext,
	sil4_transition_result_t * pxResult)
{
	vMoveToEstablished(pxContext, pxResult);
	(void)state_machine_handle_event(pxContext, SIL4_EVENT_SEQUENCE_GAP_DETECTED, pxResult);
}

static void vMoveToSafeDisconnect(
	sil4_state_machine_context_t * pxContext,
	sil4_transition_result_t * pxResult)
{
	vMoveToEstablished(pxContext, pxResult);
	(void)state_machine_handle_event(pxContext, SIL4_EVENT_DISCONNECT_REQUEST, pxResult);
}

static void vTestInitSuccess(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;
	sil4_status_t eStatus;

	eStatus = state_machine_init(&xContext);
	vAssertEqualStatus(SIL4_STATUS_OK, eStatus, "init should succeed");
	vAssertEqualState(SIL4_STATE_UNINITIALIZED, xContext.eCurrentState, "initial state");

	eStatus = state_machine_handle_event(&xContext, SIL4_EVENT_INIT_SUCCESS, &xResult);
	vAssertEqualStatus(SIL4_STATUS_OK, eStatus, "init_success event");
	vAssertEqualState(SIL4_STATE_INITIALIZED, xResult.eNextState, "state after init_success");
	vAssertActionCount(2U, xResult.xActions.uActionCount, "actions after init_success");
	vAssertActionAt(SIL4_ACTION_LOG_DIAGNOSTIC, &xResult, 0U, "init_success action 0");
	vAssertActionAt(SIL4_ACTION_NOTIFY_API, &xResult, 1U, "init_success action 1");
}

static void vTestConnectPath(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;
	sil4_status_t eStatus;

	(void)state_machine_init(&xContext);
	(void)state_machine_handle_event(&xContext, SIL4_EVENT_INIT_SUCCESS, &xResult);

	eStatus = state_machine_handle_event(&xContext, SIL4_EVENT_CONNECT_REQUEST, &xResult);
	vAssertEqualStatus(SIL4_STATUS_OK, eStatus, "connect_request event");
	vAssertEqualState(SIL4_STATE_CONNECTING, xResult.eNextState, "state after connect_request");

	eStatus = state_machine_handle_event(&xContext, SIL4_EVENT_HANDSHAKE_SUCCESS, &xResult);
	vAssertEqualStatus(SIL4_STATUS_OK, eStatus, "handshake_success event");
	vAssertEqualState(SIL4_STATE_ESTABLISHED, xResult.eNextState, "state after handshake_success");
}

static void vTestInboundConnectPath(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;
	sil4_status_t eStatus;

	vMoveToInitialized(&xContext, &xResult);

	eStatus = state_machine_handle_event(&xContext, SIL4_EVENT_VALID_INBOUND_CONNECT, &xResult);
	vAssertEqualStatus(SIL4_STATUS_OK, eStatus, "valid inbound connect event");
	vAssertEqualState(SIL4_STATE_CONNECTING, xResult.eNextState, "state after inbound connect");
	vAssertActionAt(SIL4_ACTION_ACCEPT_INBOUND_CONNECT, &xResult, 0U, "inbound connect action 0");
}

static void vTestInvalidEventFailsSafe(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;

	vMoveToConnecting(&xContext, &xResult);

	(void)state_machine_handle_event(&xContext, SIL4_EVENT_VALID_DATA, &xResult);
	vAssertEqualState(SIL4_STATE_SAFE_DISCONNECT, xResult.eNextState, "invalid connecting event should fail-safe");
	vAssertActionCount(4U, xResult.xActions.uActionCount, "fail-safe action count");
	vAssertActionAt(SIL4_ACTION_SEND_DISCONNECT, &xResult, 0U, "fail-safe action 0");
}

static void vTestInitializedRejectsUnexpectedData(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;
	sil4_status_t eStatus;

	vMoveToInitialized(&xContext, &xResult);

	eStatus = state_machine_handle_event(&xContext, SIL4_EVENT_VALID_DATA, &xResult);
	vAssertEqualStatus(SIL4_STATUS_REJECTED, eStatus, "initialized should reject valid_data");
	vAssertEqualState(SIL4_STATE_INITIALIZED, xResult.eNextState, "initialized state should be preserved");
	vAssertActionCount(2U, xResult.xActions.uActionCount, "initialized reject action count");
}

static void vTestEstablishedHeartbeatAndData(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;

	vMoveToEstablished(&xContext, &xResult);

	(void)state_machine_handle_event(&xContext, SIL4_EVENT_VALID_HEARTBEAT, &xResult);
	vAssertEqualState(SIL4_STATE_ESTABLISHED, xResult.eNextState, "heartbeat keeps established");
	vAssertActionCount(2U, xResult.xActions.uActionCount, "heartbeat action count");

	(void)state_machine_handle_event(&xContext, SIL4_EVENT_VALID_DATA, &xResult);
	vAssertEqualState(SIL4_STATE_ESTABLISHED, xResult.eNextState, "data keeps established");
	vAssertActionAt(SIL4_ACTION_DELIVER_DATA, &xResult, 1U, "data delivery action");
}

static void vTestRetransmissionPath(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;

	vMoveToRetransmissionPending(&xContext, &xResult);
	vAssertEqualState(SIL4_STATE_RETRANSMISSION_PENDING, xResult.eNextState, "state after sequence gap");

	(void)state_machine_handle_event(&xContext, SIL4_EVENT_RECOVERY_SUCCESS, &xResult);
	vAssertEqualState(SIL4_STATE_ESTABLISHED, xResult.eNextState, "recovery returns established");

	vMoveToRetransmissionPending(&xContext, &xResult);
	(void)state_machine_handle_event(&xContext, SIL4_EVENT_INVALID_RESPONSE, &xResult);
	vAssertEqualState(SIL4_STATE_SAFE_DISCONNECT, xResult.eNextState, "invalid response enters safe disconnect");
}

static void vTestTimeoutPaths(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;

	vMoveToConnecting(&xContext, &xResult);
	(void)state_machine_handle_event(&xContext, SIL4_EVENT_TIMEOUT, &xResult);
	vAssertEqualState(SIL4_STATE_SAFE_DISCONNECT, xResult.eNextState, "connecting timeout fails safe");

	vMoveToEstablished(&xContext, &xResult);
	(void)state_machine_handle_event(&xContext, SIL4_EVENT_TIMEOUT, &xResult);
	vAssertEqualState(SIL4_STATE_SAFE_DISCONNECT, xResult.eNextState, "established timeout fails safe");
}

static void vTestSafeDisconnectCleanup(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;

	vMoveToSafeDisconnect(&xContext, &xResult);
	vAssertEqualState(SIL4_STATE_SAFE_DISCONNECT, xResult.eNextState, "disconnect enters safe disconnect");

	(void)state_machine_handle_event(&xContext, SIL4_EVENT_CLEANUP_COMPLETE, &xResult);
	vAssertEqualState(SIL4_STATE_INITIALIZED, xResult.eNextState, "cleanup returns initialized");
}

static void vTestShutdownIgnoresInput(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;

	(void)state_machine_init(&xContext);
	(void)state_machine_handle_event(&xContext, SIL4_EVENT_INIT_FAILURE, &xResult);
	vAssertEqualState(SIL4_STATE_SHUTDOWN, xResult.eNextState, "init failure shutdown");

	(void)state_machine_handle_event(&xContext, SIL4_EVENT_CONNECT_REQUEST, &xResult);
	vAssertEqualState(SIL4_STATE_SHUTDOWN, xResult.eNextState, "shutdown should ignore later events");
	vAssertActionCount(0U, xResult.xActions.uActionCount, "shutdown no action");
}

static void vTestInvalidArguments(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;
	sil4_status_t eStatus;

	eStatus = state_machine_init((sil4_state_machine_context_t *)0);
	vAssertEqualStatus(SIL4_STATUS_INVALID_ARGUMENT, eStatus, "init null argument");

	(void)state_machine_init(&xContext);

	eStatus = state_machine_handle_event(&xContext, SIL4_EVENT_INVALID, &xResult);
	vAssertEqualStatus(SIL4_STATUS_INVALID_EVENT, eStatus, "invalid event enum");

	eStatus = state_machine_handle_event((sil4_state_machine_context_t *)0, SIL4_EVENT_INIT_SUCCESS, &xResult);
	vAssertEqualStatus(SIL4_STATUS_INVALID_ARGUMENT, eStatus, "handle_event null context");

	eStatus = state_machine_reset((sil4_state_machine_context_t *)0);
	vAssertEqualStatus(SIL4_STATUS_INVALID_ARGUMENT, eStatus, "reset null argument");
}

int main(void)
{
	vTestInitSuccess();
	vTestConnectPath();
	vTestInboundConnectPath();
	vTestInvalidEventFailsSafe();
	vTestInitializedRejectsUnexpectedData();
	vTestEstablishedHeartbeatAndData();
	vTestRetransmissionPath();
	vTestTimeoutPaths();
	vTestSafeDisconnectCleanup();
	vTestShutdownIgnoresInput();
	vTestInvalidArguments();

	(void)printf("sil4_state_machine_test: all tests passed\n");

	return EXIT_SUCCESS;
}
