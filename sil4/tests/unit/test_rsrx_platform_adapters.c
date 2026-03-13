#include <stdio.h>
#include <stdlib.h>

#include "rsrx_platform_adapters.h"

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
	test_clock_context_t xClockContext = { 100U, 0U };
	test_timer_context_t xTimerContext = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnosticsContext = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	test_action_context_t xTransportContext = { RSRX_ACTION_NONE, 0U };
	test_action_context_t xApiContext = { RSRX_ACTION_NONE, 0U };
	test_action_context_t xLifecycleContext = { RSRX_ACTION_NONE, 0U };
	rsrx_platform_port_table_t xPorts;
	rsrx_action_executor_t xTransportExecutor;
	rsrx_action_executor_t xApiExecutor;
	rsrx_action_executor_t xLifecycleExecutor;
	rsrx_action_executor_table_t xExecutors;
	rsrx_status_t eStatus;

	xPorts.xClock.pvContext = &xClockContext;
	xPorts.xClock.pfNow = eClockNow;
	xPorts.xTimer.pvContext = &xTimerContext;
	xPorts.xTimer.pfCommand = eTimerCommand;
	xPorts.xDiagnostics.pvContext = &xDiagnosticsContext;
	xPorts.xDiagnostics.pfWrite = eDiagnosticWrite;

	vAssertTrue(rsrx_platform_adapter_init(&xPlatformContext, &xPorts, 50U, 75U, 125U) == RSRX_PLATFORM_STATUS_OK, "platform adapter init");

	xTransportExecutor.pvContext = &xTransportContext;
	xTransportExecutor.pfDispatch = vCaptureAction;
	xApiExecutor.pvContext = &xApiContext;
	xApiExecutor.pfDispatch = vCaptureAction;
	xLifecycleExecutor.pvContext = &xLifecycleContext;
	xLifecycleExecutor.pfDispatch = vCaptureAction;

	eStatus = rsrx_platform_adapter_build_executor_table(
		&xExecutors,
		&xPlatformContext,
		&xTransportExecutor,
		&xApiExecutor,
		&xLifecycleExecutor);
	vAssertTrue(eStatus == RSRX_STATUS_OK, "build executor table");
	vAssertTrue(xExecutors.xTimerExecutor.pfDispatch == rsrx_platform_timer_executor_dispatch, "timer executor binding");
	vAssertTrue(xExecutors.xDiagnosticsExecutor.pfDispatch == rsrx_platform_diagnostics_executor_dispatch, "diagnostics executor binding");
}

static void vTestTimerAndDiagnosticsDispatch(void)
{
	rsrx_platform_adapter_context_t xPlatformContext;
	test_clock_context_t xClockContext = { 1000U, 0U };
	test_timer_context_t xTimerContext = { { RSRX_TIMER_ID_INVALID, RSRX_TIMER_COMMAND_NONE, 0U, RSRX_REASON_NONE }, 0U };
	test_diagnostics_context_t xDiagnosticsContext = { { RSRX_LOG_SEVERITY_INFO, RSRX_STATE_INVALID, RSRX_STATE_INVALID, RSRX_STATUS_OK, RSRX_REASON_NONE, RSRX_DIAG_NONE, 0U }, 0U };
	rsrx_platform_port_table_t xPorts;
	rsrx_transition_result_t xTransition;

	xPorts.xClock.pvContext = &xClockContext;
	xPorts.xClock.pfNow = eClockNow;
	xPorts.xTimer.pvContext = &xTimerContext;
	xPorts.xTimer.pfCommand = eTimerCommand;
	xPorts.xDiagnostics.pvContext = &xDiagnosticsContext;
	xPorts.xDiagnostics.pfWrite = eDiagnosticWrite;

	(void)rsrx_platform_adapter_init(&xPlatformContext, &xPorts, 200U, 300U, 400U);

	xTransition.ePreviousState = RSRX_STATE_INITIALIZED;
	xTransition.eNextState = RSRX_STATE_CONNECTING;
	xTransition.eStatus = RSRX_STATUS_OK;
	xTransition.eReason = RSRX_REASON_CONNECT_REQUESTED;
	xTransition.eDiagnostic = RSRX_DIAG_INFO_STATE_TRANSITION;
	xTransition.xActions.uActionCount = 0U;

	rsrx_platform_timer_executor_dispatch(&xPlatformContext, &xTransition, RSRX_ACTION_START_SUPERVISION_TIMER, 1U);
	vAssertTrue(xClockContext.uCallCount == 1U, "clock called");
	vAssertTrue(xTimerContext.uCallCount == 1U, "timer command called");
	vAssertTrue(xTimerContext.xLastCommand.eTimerId == RSRX_TIMER_ID_SUPERVISION, "timer id mapped");
	vAssertTrue(xTimerContext.xLastCommand.eCommandType == RSRX_TIMER_COMMAND_START, "timer command mapped");
	vAssertTrue(xTimerContext.xLastCommand.uDeadlineNs == 1200U, "timer deadline computed");

	rsrx_platform_diagnostics_executor_dispatch(&xPlatformContext, &xTransition, RSRX_ACTION_LOG_DIAGNOSTIC, 3U);
	vAssertTrue(xDiagnosticsContext.uCallCount == 1U, "diagnostics write called");
	vAssertTrue(xDiagnosticsContext.xLastRecord.eReason == RSRX_REASON_CONNECT_REQUESTED, "diagnostic reason propagated");
	vAssertTrue(xDiagnosticsContext.xLastRecord.eSeverity == RSRX_LOG_SEVERITY_INFO, "diagnostic severity mapped");
	vAssertTrue(xDiagnosticsContext.xLastRecord.uEventCounter == 1U, "diagnostic event counter incremented");
}

int main(void)
{
	vTestPlatformExecutorTableBuild();
	vTestTimerAndDiagnosticsDispatch();

	(void)printf("rsrx_platform_adapters_test: all tests passed\n");

	return EXIT_SUCCESS;
}
