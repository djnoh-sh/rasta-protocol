#ifndef RSRX_ORCHESTRATOR_H
#define RSRX_ORCHESTRATOR_H

#include <stdint.h>

#include "rsrx_state_machine.h"

typedef void (*rsrx_action_dispatch_fn)(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex);

typedef struct
{
	void * pvContext;
	rsrx_action_dispatch_fn pfDispatch;
} rsrx_action_sink_t;

typedef struct
{
	rsrx_transition_result_t xTransition;
	uint32_t uDispatchedActionCount;
} rsrx_orchestrator_report_t;

typedef struct
{
	rsrx_state_machine_context_t xStateMachine;
	rsrx_action_sink_t xActionSink;
} rsrx_orchestrator_context_t;

rsrx_status_t rsrx_orchestrator_init(
	rsrx_orchestrator_context_t * pxContext,
	const rsrx_action_sink_t * pxActionSink);

rsrx_status_t rsrx_orchestrator_process_event(
	rsrx_orchestrator_context_t * pxContext,
	rsrx_event_t eEvent,
	rsrx_orchestrator_report_t * pxReport);

rsrx_state_t rsrx_orchestrator_get_state(
	const rsrx_orchestrator_context_t * pxContext);

rsrx_status_t rsrx_orchestrator_reset(
	rsrx_orchestrator_context_t * pxContext);

#endif
