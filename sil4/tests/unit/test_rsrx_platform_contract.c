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
	xDiagnosticRecord.uEventCounter = 1U;

	xPorts.xClock.pvContext = (void *)0;
	xPorts.xClock.pfNow = (rsrx_clock_now_fn)0;
	xPorts.xTimer.pvContext = (void *)0;
	xPorts.xTimer.pfCommand = (rsrx_timer_command_fn)0;
	xPorts.xDiagnostics.pvContext = (void *)0;
	xPorts.xDiagnostics.pfWrite = (rsrx_diagnostic_write_fn)0;

	vAssertTrue(xTimerCommand.eTimerId == RSRX_TIMER_ID_SUPERVISION, "timer id contract");
	vAssertTrue(xTimerCommand.eCommandType == RSRX_TIMER_COMMAND_START, "timer command contract");
	vAssertTrue(xDiagnosticRecord.eDiagnostic == RSRX_DIAG_INFO_STATE_TRANSITION, "diagnostic contract");
	vAssertTrue(xPorts.xTimer.pfCommand == (rsrx_timer_command_fn)0, "platform table layout");

	(void)printf("rsrx_platform_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
