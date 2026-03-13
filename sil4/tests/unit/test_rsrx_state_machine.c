#include <stdio.h>
#include <stdlib.h>

#include "rsrx_state_machine.h"

static void vAssertEqualState(
	rsrx_state_t eExpected,
	rsrx_state_t eActual,
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
	rsrx_status_t eExpected,
	rsrx_status_t eActual,
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
	rsrx_action_t eExpected,
	const rsrx_transition_result_t * pxResult,
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

static void vAssertActionSequence(
	const rsrx_transition_result_t * pxResult,
	const rsrx_action_t * peExpectedActions,
	uint32_t uExpectedCount,
	const char * pcMessage)
{
	uint32_t uIndex;

	vAssertActionCount(uExpectedCount, pxResult->xActions.uActionCount, pcMessage);

	for(uIndex = 0U; uIndex < uExpectedCount; ++uIndex)
	{
		if(pxResult->xActions.eActions[uIndex] != peExpectedActions[uIndex])
		{
			(void)fprintf(
				stderr,
				"ASSERT FAILED: %s (index=%u expected=%d actual=%d)\n",
				pcMessage,
				(unsigned int)uIndex,
				(int)peExpectedActions[uIndex],
				(int)pxResult->xActions.eActions[uIndex]);
			exit(EXIT_FAILURE);
		}
	}
}

static void vAssertNoDuplicateActions(
	const rsrx_transition_result_t * pxResult,
	const char * pcMessage)
{
	uint32_t uOuterIndex;
	uint32_t uInnerIndex;

	for(uOuterIndex = 0U; uOuterIndex < pxResult->xActions.uActionCount; ++uOuterIndex)
	{
		for(uInnerIndex = uOuterIndex + 1U;
			uInnerIndex < pxResult->xActions.uActionCount;
			++uInnerIndex)
		{
			if(pxResult->xActions.eActions[uOuterIndex] ==
				pxResult->xActions.eActions[uInnerIndex])
			{
				(void)fprintf(
					stderr,
					"ASSERT FAILED: %s (duplicate_action=%d first_index=%u second_index=%u)\n",
					pcMessage,
					(int)pxResult->xActions.eActions[uOuterIndex],
					(unsigned int)uOuterIndex,
					(unsigned int)uInnerIndex);
				exit(EXIT_FAILURE);
			}
		}
	}
}

static void vMoveToInitialized(
	rsrx_state_machine_context_t * pxContext,
	rsrx_transition_result_t * pxResult)
{
	(void)rsrx_state_machine_init(pxContext);
	(void)rsrx_state_machine_handle_event(pxContext, RSRX_EVENT_INIT_SUCCESS, pxResult);
}

static void vMoveToConnecting(
	rsrx_state_machine_context_t * pxContext,
	rsrx_transition_result_t * pxResult)
{
	vMoveToInitialized(pxContext, pxResult);
	(void)rsrx_state_machine_handle_event(pxContext, RSRX_EVENT_CONNECT_REQUEST, pxResult);
}

static void vMoveToEstablished(
	rsrx_state_machine_context_t * pxContext,
	rsrx_transition_result_t * pxResult)
{
	vMoveToConnecting(pxContext, pxResult);
	(void)rsrx_state_machine_handle_event(pxContext, RSRX_EVENT_HANDSHAKE_SUCCESS, pxResult);
}

static void vMoveToRetransmissionPending(
	rsrx_state_machine_context_t * pxContext,
	rsrx_transition_result_t * pxResult)
{
	vMoveToEstablished(pxContext, pxResult);
	(void)rsrx_state_machine_handle_event(pxContext, RSRX_EVENT_SEQUENCE_GAP_DETECTED, pxResult);
}

static void vMoveToSafeDisconnect(
	rsrx_state_machine_context_t * pxContext,
	rsrx_transition_result_t * pxResult)
{
	vMoveToEstablished(pxContext, pxResult);
	(void)rsrx_state_machine_handle_event(pxContext, RSRX_EVENT_DISCONNECT_REQUEST, pxResult);
}

static void vTestInitSuccess(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;
	rsrx_status_t eStatus;

	eStatus = rsrx_state_machine_init(&xContext);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "init should succeed");
	vAssertEqualState(RSRX_STATE_UNINITIALIZED, xContext.eCurrentState, "initial state");

	eStatus = rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_INIT_SUCCESS, &xResult);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "init_success event");
	vAssertEqualState(RSRX_STATE_INITIALIZED, xResult.eNextState, "state after init_success");
	vAssertActionCount(2U, xResult.xActions.uActionCount, "actions after init_success");
	vAssertActionAt(RSRX_ACTION_LOG_DIAGNOSTIC, &xResult, 0U, "init_success action 0");
	vAssertActionAt(RSRX_ACTION_NOTIFY_API, &xResult, 1U, "init_success action 1");
}

static void vTestConnectPath(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;
	rsrx_status_t eStatus;
	static const rsrx_action_t xeExpectedConnectActions[] = {
		RSRX_ACTION_START_HANDSHAKE,
		RSRX_ACTION_START_SUPERVISION_TIMER,
		RSRX_ACTION_NOTIFY_API
	};

	(void)rsrx_state_machine_init(&xContext);
	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_INIT_SUCCESS, &xResult);

	eStatus = rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_CONNECT_REQUEST, &xResult);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "connect_request event");
	vAssertEqualState(RSRX_STATE_CONNECTING, xResult.eNextState, "state after connect_request");
	vAssertActionSequence(
		&xResult,
		xeExpectedConnectActions,
		(uint32_t)(sizeof(xeExpectedConnectActions) / sizeof(xeExpectedConnectActions[0])),
		"connect_request actions");
	vAssertNoDuplicateActions(&xResult, "connect_request actions must be unique");

	eStatus = rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_HANDSHAKE_SUCCESS, &xResult);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "handshake_success event");
	vAssertEqualState(RSRX_STATE_ESTABLISHED, xResult.eNextState, "state after handshake_success");
}

static void vTestInboundConnectPath(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;
	rsrx_status_t eStatus;

	vMoveToInitialized(&xContext, &xResult);

	eStatus = rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_VALID_INBOUND_CONNECT, &xResult);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "valid inbound connect event");
	vAssertEqualState(RSRX_STATE_CONNECTING, xResult.eNextState, "state after inbound connect");
	vAssertActionAt(RSRX_ACTION_ACCEPT_INBOUND_CONNECT, &xResult, 0U, "inbound connect action 0");
}

static void vTestInvalidEventFailsSafe(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;
	static const rsrx_action_t xeExpectedFailSafeActions[] = {
		RSRX_ACTION_SEND_DISCONNECT,
		RSRX_ACTION_ENTER_FAILSAFE,
		RSRX_ACTION_NOTIFY_API,
		RSRX_ACTION_LOG_DIAGNOSTIC
	};

	vMoveToConnecting(&xContext, &xResult);

	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_VALID_DATA, &xResult);
	vAssertEqualState(RSRX_STATE_SAFE_DISCONNECT, xResult.eNextState, "invalid connecting event should fail-safe");
	vAssertActionSequence(
		&xResult,
		xeExpectedFailSafeActions,
		(uint32_t)(sizeof(xeExpectedFailSafeActions) / sizeof(xeExpectedFailSafeActions[0])),
		"fail-safe action order");
	vAssertNoDuplicateActions(&xResult, "fail-safe actions must be unique");
}

static void vTestInitializedRejectsUnexpectedData(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;
	rsrx_status_t eStatus;

	vMoveToInitialized(&xContext, &xResult);

	eStatus = rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_VALID_DATA, &xResult);
	vAssertEqualStatus(RSRX_STATUS_REJECTED, eStatus, "initialized should reject valid_data");
	vAssertEqualState(RSRX_STATE_INITIALIZED, xResult.eNextState, "initialized state should be preserved");
	vAssertActionCount(2U, xResult.xActions.uActionCount, "initialized reject action count");
}

static void vTestEstablishedHeartbeatAndData(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;

	vMoveToEstablished(&xContext, &xResult);

	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_VALID_HEARTBEAT, &xResult);
	vAssertEqualState(RSRX_STATE_ESTABLISHED, xResult.eNextState, "heartbeat keeps established");
	vAssertActionCount(2U, xResult.xActions.uActionCount, "heartbeat action count");

	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_VALID_DATA, &xResult);
	vAssertEqualState(RSRX_STATE_ESTABLISHED, xResult.eNextState, "data keeps established");
	vAssertActionAt(RSRX_ACTION_DELIVER_DATA, &xResult, 1U, "data delivery action");
}

static void vTestRetransmissionPath(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;

	vMoveToRetransmissionPending(&xContext, &xResult);
	vAssertEqualState(RSRX_STATE_RETRANSMISSION_PENDING, xResult.eNextState, "state after sequence gap");

	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_RECOVERY_SUCCESS, &xResult);
	vAssertEqualState(RSRX_STATE_ESTABLISHED, xResult.eNextState, "recovery returns established");

	vMoveToRetransmissionPending(&xContext, &xResult);
	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_INVALID_RESPONSE, &xResult);
	vAssertEqualState(RSRX_STATE_SAFE_DISCONNECT, xResult.eNextState, "invalid response enters safe disconnect");
}

static void vTestTimeoutPaths(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;

	vMoveToConnecting(&xContext, &xResult);
	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_TIMEOUT, &xResult);
	vAssertEqualState(RSRX_STATE_SAFE_DISCONNECT, xResult.eNextState, "connecting timeout fails safe");

	vMoveToEstablished(&xContext, &xResult);
	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_TIMEOUT, &xResult);
	vAssertEqualState(RSRX_STATE_SAFE_DISCONNECT, xResult.eNextState, "established timeout fails safe");
}

static void vTestSafeDisconnectCleanup(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;

	vMoveToSafeDisconnect(&xContext, &xResult);
	vAssertEqualState(RSRX_STATE_SAFE_DISCONNECT, xResult.eNextState, "disconnect enters safe disconnect");

	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_CLEANUP_COMPLETE, &xResult);
	vAssertEqualState(RSRX_STATE_INITIALIZED, xResult.eNextState, "cleanup returns initialized");
}

static void vTestActionOrderingAndUniqueness(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;
	static const rsrx_action_t xeExpectedShutdownActions[] = {
		RSRX_ACTION_SEND_DISCONNECT,
		RSRX_ACTION_RELEASE_CONNECTION_RESOURCES,
		RSRX_ACTION_FINALIZE_SHUTDOWN,
		RSRX_ACTION_NOTIFY_API
	};

	vMoveToEstablished(&xContext, &xResult);
	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_TIMEOUT, &xResult);
	vAssertNoDuplicateActions(&xResult, "timeout fail-safe actions must be unique");

	vMoveToEstablished(&xContext, &xResult);
	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_SHUTDOWN_REQUEST, &xResult);
	vAssertActionSequence(
		&xResult,
		xeExpectedShutdownActions,
		(uint32_t)(sizeof(xeExpectedShutdownActions) / sizeof(xeExpectedShutdownActions[0])),
		"shutdown action order");
	vAssertNoDuplicateActions(&xResult, "shutdown actions must be unique");
}

static void vTestShutdownIgnoresInput(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;

	(void)rsrx_state_machine_init(&xContext);
	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_INIT_FAILURE, &xResult);
	vAssertEqualState(RSRX_STATE_SHUTDOWN, xResult.eNextState, "init failure shutdown");

	(void)rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_CONNECT_REQUEST, &xResult);
	vAssertEqualState(RSRX_STATE_SHUTDOWN, xResult.eNextState, "shutdown should ignore later events");
	vAssertActionCount(0U, xResult.xActions.uActionCount, "shutdown no action");
}

static void vTestInvalidArguments(void)
{
	rsrx_state_machine_context_t xContext;
	rsrx_transition_result_t xResult;
	rsrx_status_t eStatus;

	eStatus = rsrx_state_machine_init((rsrx_state_machine_context_t *)0);
	vAssertEqualStatus(RSRX_STATUS_INVALID_ARGUMENT, eStatus, "init null argument");

	(void)rsrx_state_machine_init(&xContext);

	eStatus = rsrx_state_machine_handle_event(&xContext, RSRX_EVENT_INVALID, &xResult);
	vAssertEqualStatus(RSRX_STATUS_INVALID_EVENT, eStatus, "invalid event enum");

	eStatus = rsrx_state_machine_handle_event((rsrx_state_machine_context_t *)0, RSRX_EVENT_INIT_SUCCESS, &xResult);
	vAssertEqualStatus(RSRX_STATUS_INVALID_ARGUMENT, eStatus, "handle_event null context");

	eStatus = rsrx_state_machine_reset((rsrx_state_machine_context_t *)0);
	vAssertEqualStatus(RSRX_STATUS_INVALID_ARGUMENT, eStatus, "reset null argument");
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
	vTestActionOrderingAndUniqueness();
	vTestShutdownIgnoresInput();
	vTestInvalidArguments();

	(void)printf("rsrx_state_machine_test: all tests passed\n");

	return EXIT_SUCCESS;
}
