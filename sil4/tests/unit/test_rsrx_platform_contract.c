#include <stdio.h>
#include <stdlib.h>

#include "rsrx_platform.h"

static void vAssertTrue(int iCondition, const char * pcMessage)
{
	if(iCondition == 0)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s\n", pcMessage);
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	uint32_t uExpectedEventCounter;
	rsrx_timer_command_t xTimerCommand;
	rsrx_diagnostic_record_t xDiagnosticRecord;
	rsrx_platform_port_table_t xPorts;

	xTimerCommand.eTimerId = RSRX_TIMER_ID_SUPERVISION;
	xTimerCommand.eCommandType = RSRX_TIMER_COMMAND_START;
	xTimerCommand.uDeadlineNs = 1000U;
	xTimerCommand.eReason = RSRX_REASON_CONNECT_REQUESTED;

	xDiagnosticRecord.eSeverity = RSRX_LOG_SEVERITY_WARNING;
	xDiagnosticRecord.ePreviousState = RSRX_STATE_INITIALIZED;
	xDiagnosticRecord.eNextState = RSRX_STATE_CONNECTING;
	xDiagnosticRecord.eStatus = RSRX_STATUS_OK;
	xDiagnosticRecord.eReason = RSRX_REASON_CONNECT_REQUESTED;
	xDiagnosticRecord.eDiagnostic = RSRX_DIAG_INFO_STATE_TRANSITION;
	uExpectedEventCounter = 1U;
	xDiagnosticRecord.uEventCounter = uExpectedEventCounter;

	xPorts.xClock.pvContext = (void *)0;
	xPorts.xClock.pfNow = (rsrx_clock_now_fn)0;
	xPorts.xTimer.pvContext = (void *)0;
	xPorts.xTimer.pfCommand = (rsrx_timer_command_fn)0;
	xPorts.xDiagnostics.pvContext = (void *)0;
	xPorts.xDiagnostics.pfWrite = (rsrx_diagnostic_write_fn)0;
	xPorts.xCriticalSection.pvContext = (void *)0;
	xPorts.xCriticalSection.pfEnter = (rsrx_critical_section_enter_fn)0;
	xPorts.xCriticalSection.pfExit = (rsrx_critical_section_exit_fn)0;

	vAssertTrue(xTimerCommand.eTimerId == RSRX_TIMER_ID_SUPERVISION, "timer id contract");
	vAssertTrue(xTimerCommand.eCommandType == RSRX_TIMER_COMMAND_START, "timer command contract");
	vAssertTrue(xTimerCommand.uDeadlineNs == 1000U, "timer deadline contract");
	vAssertTrue(xTimerCommand.eReason == RSRX_REASON_CONNECT_REQUESTED, "timer reason contract");
	vAssertTrue(xDiagnosticRecord.eSeverity == RSRX_LOG_SEVERITY_WARNING, "diagnostic severity contract");
	vAssertTrue(xDiagnosticRecord.ePreviousState == RSRX_STATE_INITIALIZED, "diagnostic previous state contract");
	vAssertTrue(xDiagnosticRecord.eNextState == RSRX_STATE_CONNECTING, "diagnostic next state contract");
	vAssertTrue(xDiagnosticRecord.eStatus == RSRX_STATUS_OK, "diagnostic status contract");
	vAssertTrue(xDiagnosticRecord.eReason == RSRX_REASON_CONNECT_REQUESTED, "diagnostic reason contract");
	vAssertTrue(xDiagnosticRecord.eDiagnostic == RSRX_DIAG_INFO_STATE_TRANSITION, "diagnostic contract");
	/* cppcheck-suppress knownConditionTrueFalse */
	vAssertTrue(xDiagnosticRecord.uEventCounter == uExpectedEventCounter, "diagnostic event counter contract");
	vAssertTrue(xPorts.xTimer.pfCommand == (rsrx_timer_command_fn)0, "platform table layout");
	vAssertTrue(
		xPorts.xCriticalSection.pfEnter == (rsrx_critical_section_enter_fn)0,
		"critical section enter layout");

	(void)printf("rsrx_platform_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
