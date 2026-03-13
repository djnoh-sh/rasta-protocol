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

static void vTestInvalidEventFailsSafe(void)
{
	sil4_state_machine_context_t xContext;
	sil4_transition_result_t xResult;

	(void)state_machine_init(&xContext);
	(void)state_machine_handle_event(&xContext, SIL4_EVENT_INIT_SUCCESS, &xResult);
	(void)state_machine_handle_event(&xContext, SIL4_EVENT_CONNECT_REQUEST, &xResult);

	(void)state_machine_handle_event(&xContext, SIL4_EVENT_VALID_DATA, &xResult);
	vAssertEqualState(SIL4_STATE_SAFE_DISCONNECT, xResult.eNextState, "invalid connecting event should fail-safe");
	vAssertActionCount(4U, xResult.xActions.uActionCount, "fail-safe action count");
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

int main(void)
{
	vTestInitSuccess();
	vTestConnectPath();
	vTestInvalidEventFailsSafe();
	vTestShutdownIgnoresInput();

	(void)printf("sil4_state_machine_test: all tests passed\n");

	return EXIT_SUCCESS;
}

