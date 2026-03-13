#ifndef RSRX_PLATFORM_H
#define RSRX_PLATFORM_H

#include <stdint.h>

#include "rsrx_state_machine.h"

typedef uint64_t rsrx_monotonic_time_ns_t;

typedef enum
{
	RSRX_PLATFORM_STATUS_OK = 0,
	RSRX_PLATFORM_STATUS_INVALID_ARGUMENT,
	RSRX_PLATFORM_STATUS_UNAVAILABLE,
	RSRX_PLATFORM_STATUS_IO_ERROR,
	RSRX_PLATFORM_STATUS_INTERNAL_ERROR
} rsrx_platform_status_t;

typedef enum
{
	RSRX_TIMER_ID_INVALID = 0,
	RSRX_TIMER_ID_SUPERVISION,
	RSRX_TIMER_ID_RETRANSMISSION,
	RSRX_TIMER_ID_DIAGNOSTIC_FLUSH
} rsrx_timer_id_t;

typedef enum
{
	RSRX_TIMER_COMMAND_NONE = 0,
	RSRX_TIMER_COMMAND_START,
	RSRX_TIMER_COMMAND_RESTART,
	RSRX_TIMER_COMMAND_CANCEL
} rsrx_timer_command_type_t;

typedef enum
{
	RSRX_LOG_SEVERITY_INFO = 0,
	RSRX_LOG_SEVERITY_WARNING,
	RSRX_LOG_SEVERITY_ERROR,
	RSRX_LOG_SEVERITY_FATAL
} rsrx_log_severity_t;

typedef struct
{
	rsrx_timer_id_t eTimerId;
	rsrx_timer_command_type_t eCommandType;
	rsrx_monotonic_time_ns_t uDeadlineNs;
	rsrx_reason_code_t eReason;
} rsrx_timer_command_t;

typedef struct
{
	rsrx_log_severity_t eSeverity;
	rsrx_state_t ePreviousState;
	rsrx_state_t eNextState;
	rsrx_status_t eStatus;
	rsrx_reason_code_t eReason;
	rsrx_diagnostic_code_t eDiagnostic;
	uint32_t uEventCounter;
} rsrx_diagnostic_record_t;

typedef rsrx_platform_status_t (*rsrx_clock_now_fn)(
	void * pvContext,
	rsrx_monotonic_time_ns_t * puNowNs);

typedef rsrx_platform_status_t (*rsrx_timer_command_fn)(
	void * pvContext,
	const rsrx_timer_command_t * pxCommand);

typedef rsrx_platform_status_t (*rsrx_diagnostic_write_fn)(
	void * pvContext,
	const rsrx_diagnostic_record_t * pxRecord);

typedef struct
{
	void * pvContext;
	rsrx_clock_now_fn pfNow;
} rsrx_clock_port_t;

typedef struct
{
	void * pvContext;
	rsrx_timer_command_fn pfCommand;
} rsrx_timer_port_t;

typedef struct
{
	void * pvContext;
	rsrx_diagnostic_write_fn pfWrite;
} rsrx_diagnostics_port_t;

typedef struct
{
	rsrx_clock_port_t xClock;
	rsrx_timer_port_t xTimer;
	rsrx_diagnostics_port_t xDiagnostics;
} rsrx_platform_port_table_t;

#endif
