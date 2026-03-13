#ifndef RSRX_STATE_MACHINE_H
#define RSRX_STATE_MACHINE_H

#include <stdint.h>

#define D_RSRX_ACTION_CAPACITY (4U)

typedef enum
{
	RSRX_STATE_UNINITIALIZED = 0,
	RSRX_STATE_INITIALIZED,
	RSRX_STATE_CONNECTING,
	RSRX_STATE_ESTABLISHED,
	RSRX_STATE_RETRANSMISSION_PENDING,
	RSRX_STATE_SAFE_DISCONNECT,
	RSRX_STATE_SHUTDOWN,
	RSRX_STATE_INVALID
} rsrx_state_t;

typedef enum
{
	RSRX_EVENT_INIT_SUCCESS = 0,
	RSRX_EVENT_INIT_FAILURE,
	RSRX_EVENT_CONNECT_REQUEST,
	RSRX_EVENT_VALID_INBOUND_CONNECT,
	RSRX_EVENT_HANDSHAKE_SUCCESS,
	RSRX_EVENT_VALID_HEARTBEAT,
	RSRX_EVENT_VALID_DATA,
	RSRX_EVENT_SEQUENCE_GAP_DETECTED,
	RSRX_EVENT_RECOVERY_SUCCESS,
	RSRX_EVENT_DISCONNECT_REQUEST,
	RSRX_EVENT_TIMEOUT,
	RSRX_EVENT_INVALID_MESSAGE,
	RSRX_EVENT_VERSION_MISMATCH,
	RSRX_EVENT_PROTOCOL_ERROR,
	RSRX_EVENT_RETRANSMISSION_FAILURE,
	RSRX_EVENT_INVALID_RESPONSE,
	RSRX_EVENT_SHUTDOWN_REQUEST,
	RSRX_EVENT_CLEANUP_COMPLETE,
	RSRX_EVENT_INVALID
} rsrx_event_t;

typedef enum
{
	RSRX_ACTION_NONE = 0,
	RSRX_ACTION_START_HANDSHAKE,
	RSRX_ACTION_ACCEPT_INBOUND_CONNECT,
	RSRX_ACTION_START_SUPERVISION_TIMER,
	RSRX_ACTION_RESET_SUPERVISION_TIMER,
	RSRX_ACTION_SEND_HEARTBEAT,
	RSRX_ACTION_DELIVER_DATA,
	RSRX_ACTION_REQUEST_RETRANSMISSION,
	RSRX_ACTION_CLEAR_RETRANSMISSION_CONTEXT,
	RSRX_ACTION_SEND_DISCONNECT,
	RSRX_ACTION_ENTER_FAILSAFE,
	RSRX_ACTION_LOG_DIAGNOSTIC,
	RSRX_ACTION_RELEASE_CONNECTION_RESOURCES,
	RSRX_ACTION_NOTIFY_API,
	RSRX_ACTION_FINALIZE_SHUTDOWN
} rsrx_action_t;

typedef enum
{
	RSRX_STATUS_OK = 0,
	RSRX_STATUS_INVALID_ARGUMENT,
	RSRX_STATUS_INVALID_EVENT,
	RSRX_STATUS_INVALID_STATE,
	RSRX_STATUS_REJECTED
} rsrx_status_t;

typedef struct
{
	rsrx_action_t eActions[D_RSRX_ACTION_CAPACITY];
	uint32_t uActionCount;
} rsrx_action_list_t;

typedef struct
{
	rsrx_state_t eCurrentState;
	rsrx_status_t eLastStatus;
	uint32_t uEventCounter;
} rsrx_state_machine_context_t;

typedef struct
{
	rsrx_state_t ePreviousState;
	rsrx_state_t eNextState;
	rsrx_status_t eStatus;
	rsrx_action_list_t xActions;
} rsrx_transition_result_t;

rsrx_status_t rsrx_state_machine_init(
	rsrx_state_machine_context_t * pxContext);

rsrx_status_t rsrx_state_machine_handle_event(
	rsrx_state_machine_context_t * pxContext,
	rsrx_event_t eEvent,
	rsrx_transition_result_t * pxResult);

rsrx_state_t rsrx_state_machine_get_state(
	const rsrx_state_machine_context_t * pxContext);

rsrx_status_t rsrx_state_machine_reset(
	rsrx_state_machine_context_t * pxContext);

#endif

