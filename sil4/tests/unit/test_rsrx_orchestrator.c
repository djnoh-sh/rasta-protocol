#include <stdio.h>
#include <stdlib.h>

#include "rsrx_orchestrator.h"

typedef struct
{
	rsrx_action_t eActions[16];
	uint32_t uActionCount;
	rsrx_reason_code_t eLastReason;
	rsrx_diagnostic_code_t eLastDiagnostic;
} test_sink_context_t;

static void vAssertEqualUint32(uint32_t uExpected, uint32_t uActual, const char * pcMessage)
{
	if(uExpected != uActual)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s (expected=%u actual=%u)\n", pcMessage, (unsigned int)uExpected, (unsigned int)uActual);
		exit(EXIT_FAILURE);
	}
}

static void vAssertEqualState(rsrx_state_t eExpected, rsrx_state_t eActual, const char * pcMessage)
{
	if(eExpected != eActual)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s (expected=%d actual=%d)\n", pcMessage, (int)eExpected, (int)eActual);
		exit(EXIT_FAILURE);
	}
}

static void vAssertEqualStatus(rsrx_status_t eExpected, rsrx_status_t eActual, const char * pcMessage)
{
	if(eExpected != eActual)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s (expected=%d actual=%d)\n", pcMessage, (int)eExpected, (int)eActual);
		exit(EXIT_FAILURE);
	}
}

static void vAssertEqualAction(rsrx_action_t eExpected, rsrx_action_t eActual, const char * pcMessage)
{
	if(eExpected != eActual)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s (expected=%d actual=%d)\n", pcMessage, (int)eExpected, (int)eActual);
		exit(EXIT_FAILURE);
	}
}

static void vAssertEqualReason(rsrx_reason_code_t eExpected, rsrx_reason_code_t eActual, const char * pcMessage)
{
	if(eExpected != eActual)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s (expected=%d actual=%d)\n", pcMessage, (int)eExpected, (int)eActual);
		exit(EXIT_FAILURE);
	}
}

static void vDispatchAction(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	test_sink_context_t * pxSinkContext = (test_sink_context_t *)pvContext;

	if(uActionIndex < 16U)
	{
		pxSinkContext->eActions[uActionIndex] = eAction;
	}

	pxSinkContext->uActionCount++;
	pxSinkContext->eLastReason = pxTransition->eReason;
	pxSinkContext->eLastDiagnostic = pxTransition->eDiagnostic;
}

static void vTestInitAndConnectDispatch(void)
{
	rsrx_orchestrator_context_t xContext;
	rsrx_orchestrator_report_t xReport;
	test_sink_context_t xSinkContext = { { RSRX_ACTION_NONE }, 0U, RSRX_REASON_NONE, RSRX_DIAG_NONE };
	rsrx_action_sink_t xSink = { &xSinkContext, vDispatchAction };
	rsrx_status_t eStatus;

	eStatus = rsrx_orchestrator_init(&xContext, &xSink);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "orchestrator init");
	vAssertEqualState(RSRX_STATE_UNINITIALIZED, rsrx_orchestrator_get_state(&xContext), "initial orchestrator state");

	eStatus = rsrx_orchestrator_process_event(&xContext, RSRX_EVENT_INIT_SUCCESS, &xReport);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "init success status");
	vAssertEqualUint32(2U, xReport.uDispatchedActionCount, "init success dispatched count");
	vAssertEqualReason(RSRX_REASON_INIT_COMPLETED, xReport.xTransition.eReason, "init success reason");

	xSinkContext.uActionCount = 0U;
	eStatus = rsrx_orchestrator_process_event(&xContext, RSRX_EVENT_CONNECT_REQUEST, &xReport);
	vAssertEqualStatus(RSRX_STATUS_OK, eStatus, "connect request status");
	vAssertEqualState(RSRX_STATE_CONNECTING, rsrx_orchestrator_get_state(&xContext), "connecting state");
	vAssertEqualUint32(3U, xReport.uDispatchedActionCount, "connect dispatched count");
	vAssertEqualUint32(3U, xSinkContext.uActionCount, "sink action count");
	vAssertEqualAction(RSRX_ACTION_START_HANDSHAKE, xSinkContext.eActions[0], "connect action 0");
	vAssertEqualAction(RSRX_ACTION_START_SUPERVISION_TIMER, xSinkContext.eActions[1], "connect action 1");
	vAssertEqualAction(RSRX_ACTION_NOTIFY_API, xSinkContext.eActions[2], "connect action 2");
	vAssertEqualReason(RSRX_REASON_CONNECT_REQUESTED, xSinkContext.eLastReason, "sink sees connect reason");
}

static void vTestFailSafeDispatch(void)
{
	rsrx_orchestrator_context_t xContext;
	rsrx_orchestrator_report_t xReport;
	test_sink_context_t xSinkContext = { { RSRX_ACTION_NONE }, 0U, RSRX_REASON_NONE, RSRX_DIAG_NONE };
	rsrx_action_sink_t xSink = { &xSinkContext, vDispatchAction };

	(void)rsrx_orchestrator_init(&xContext, &xSink);
	(void)rsrx_orchestrator_process_event(&xContext, RSRX_EVENT_INIT_SUCCESS, &xReport);
	xSinkContext.uActionCount = 0U;
	(void)rsrx_orchestrator_process_event(&xContext, RSRX_EVENT_CONNECT_REQUEST, &xReport);
	xSinkContext.uActionCount = 0U;

	(void)rsrx_orchestrator_process_event(&xContext, RSRX_EVENT_VALID_DATA, &xReport);
	vAssertEqualState(RSRX_STATE_SAFE_DISCONNECT, rsrx_orchestrator_get_state(&xContext), "fail-safe state");
	vAssertEqualUint32(4U, xReport.uDispatchedActionCount, "fail-safe dispatched count");
	vAssertEqualReason(RSRX_REASON_CONSERVATIVE_FAILSAFE, xReport.xTransition.eReason, "fail-safe reason");
	vAssertEqualAction(RSRX_ACTION_SEND_DISCONNECT, xSinkContext.eActions[0], "fail-safe action 0");
	vAssertEqualAction(RSRX_ACTION_ENTER_FAILSAFE, xSinkContext.eActions[1], "fail-safe action 1");
}

static void vTestInvalidArguments(void)
{
	rsrx_orchestrator_report_t xReport;
	rsrx_status_t eStatus;
	rsrx_orchestrator_context_t xContext;
	rsrx_action_sink_t xInvalidSink = { (void *)0, (rsrx_action_dispatch_fn)0 };

	eStatus = rsrx_orchestrator_init((rsrx_orchestrator_context_t *)0, &xInvalidSink);
	vAssertEqualStatus(RSRX_STATUS_INVALID_ARGUMENT, eStatus, "null orchestrator context");

	eStatus = rsrx_orchestrator_init(&xContext, &xInvalidSink);
	vAssertEqualStatus(RSRX_STATUS_INVALID_ARGUMENT, eStatus, "invalid sink");

	eStatus = rsrx_orchestrator_process_event(&xContext, RSRX_EVENT_INIT_SUCCESS, &xReport);
	vAssertEqualStatus(RSRX_STATUS_INVALID_ARGUMENT, eStatus, "process event without init");
	vAssertEqualReason(RSRX_REASON_INVALID_INPUT_ARGUMENT, xReport.xTransition.eReason, "invalid argument reason");
}

int main(void)
{
	vTestInitAndConnectDispatch();
	vTestFailSafeDispatch();
	vTestInvalidArguments();

	(void)printf("rsrx_orchestrator_test: all tests passed\n");

	return EXIT_SUCCESS;
}
