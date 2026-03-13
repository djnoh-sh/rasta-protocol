#include "rsrx_state_machine.h"

typedef struct
{
	rsrx_state_t eCurrentState;
	rsrx_event_t eEvent;
	rsrx_state_t eNextState;
	rsrx_status_t eStatus;
	rsrx_reason_code_t eReason;
	rsrx_diagnostic_code_t eDiagnostic;
	rsrx_action_t eActions[D_RSRX_ACTION_CAPACITY];
	uint32_t uActionCount;
} rsrx_transition_rule_t;

static const rsrx_action_t xeRejectActions[2] =
{
	RSRX_ACTION_LOG_DIAGNOSTIC,
	RSRX_ACTION_NOTIFY_API
};

static const rsrx_action_t xeFailSafeActions[D_RSRX_ACTION_CAPACITY] =
{
	RSRX_ACTION_SEND_DISCONNECT,
	RSRX_ACTION_ENTER_FAILSAFE,
	RSRX_ACTION_NOTIFY_API,
	RSRX_ACTION_LOG_DIAGNOSTIC
};

static void vSetNoActionResult(
	rsrx_transition_result_t * pxResult,
	rsrx_state_t eCurrentState,
	rsrx_status_t eStatus,
	rsrx_reason_code_t eReason,
	rsrx_diagnostic_code_t eDiagnostic)
{
	uint32_t uIndex;

	pxResult->ePreviousState = eCurrentState;
	pxResult->eNextState = eCurrentState;
	pxResult->eStatus = eStatus;
	pxResult->eReason = eReason;
	pxResult->eDiagnostic = eDiagnostic;
	pxResult->xActions.uActionCount = 0U;

	for(uIndex = 0U; uIndex < D_RSRX_ACTION_CAPACITY; ++uIndex)
	{
		pxResult->xActions.eActions[uIndex] = RSRX_ACTION_NONE;
	}
}

static void vCopyActions(
	rsrx_action_list_t * pxDestination,
	const rsrx_action_t * peSourceActions,
	uint32_t uActionCount)
{
	uint32_t uIndex;

	pxDestination->uActionCount = uActionCount;

	for(uIndex = 0U; uIndex < D_RSRX_ACTION_CAPACITY; ++uIndex)
	{
		if(uIndex < uActionCount)
		{
			pxDestination->eActions[uIndex] = peSourceActions[uIndex];
		}
		else
		{
			pxDestination->eActions[uIndex] = RSRX_ACTION_NONE;
		}
	}
}

static void vApplyTransitionRule(
	rsrx_transition_result_t * pxResult,
	rsrx_state_machine_context_t * pxContext,
	const rsrx_transition_rule_t * pxRule)
{
	pxResult->ePreviousState = pxContext->eCurrentState;
	pxResult->eNextState = pxRule->eNextState;
	pxResult->eStatus = pxRule->eStatus;
	pxResult->eReason = pxRule->eReason;
	pxResult->eDiagnostic = pxRule->eDiagnostic;

	vCopyActions(
		&pxResult->xActions,
		pxRule->eActions,
		pxRule->uActionCount);

	pxContext->eCurrentState = pxRule->eNextState;
	pxContext->eLastStatus = pxRule->eStatus;
	pxContext->eLastReason = pxRule->eReason;
	pxContext->eLastDiagnostic = pxRule->eDiagnostic;
}

static uint32_t uIsStateValid(
	rsrx_state_t eState)
{
	return (uint32_t)((eState >= RSRX_STATE_UNINITIALIZED) &&
		(eState < RSRX_STATE_INVALID));
}

static uint32_t uIsEventValid(
	rsrx_event_t eEvent)
{
	return (uint32_t)((eEvent >= RSRX_EVENT_INIT_SUCCESS) &&
		(eEvent < RSRX_EVENT_INVALID));
}

static uint32_t uStateUsesConservativeFailSafePolicy(
	rsrx_state_t eState)
{
	return (uint32_t)((eState == RSRX_STATE_CONNECTING) ||
		(eState == RSRX_STATE_ESTABLISHED) ||
		(eState == RSRX_STATE_RETRANSMISSION_PENDING));
}

static const rsrx_transition_rule_t * pxFindTransitionRule(
	rsrx_state_t eState,
	rsrx_event_t eEvent);

static void vBuildSafeDisconnectResult(
	rsrx_transition_result_t * pxResult,
	rsrx_state_machine_context_t * pxContext);

static void vBuildIgnoredSafeDisconnectResult(
	rsrx_transition_result_t * pxResult,
	rsrx_state_machine_context_t * pxContext);

static const rsrx_transition_rule_t xTransitionRules[] =
{
	{
		RSRX_STATE_UNINITIALIZED,
		RSRX_EVENT_INIT_SUCCESS,
		RSRX_STATE_INITIALIZED,
		RSRX_STATUS_OK,
		RSRX_REASON_INIT_COMPLETED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_NONE, RSRX_ACTION_NONE },
		2U
	},
	{
		RSRX_STATE_UNINITIALIZED,
		RSRX_EVENT_INIT_FAILURE,
		RSRX_STATE_SHUTDOWN,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_INIT_FAILED,
		RSRX_DIAG_ERROR_CONFIGURATION,
		{ RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_FINALIZE_SHUTDOWN, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_UNINITIALIZED,
		RSRX_EVENT_SHUTDOWN_REQUEST,
		RSRX_STATE_SHUTDOWN,
		RSRX_STATUS_OK,
		RSRX_REASON_SHUTDOWN_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_FINALIZE_SHUTDOWN, RSRX_ACTION_NONE, RSRX_ACTION_NONE, RSRX_ACTION_NONE },
		1U
	},
	{
		RSRX_STATE_INITIALIZED,
		RSRX_EVENT_CONNECT_REQUEST,
		RSRX_STATE_CONNECTING,
		RSRX_STATUS_OK,
		RSRX_REASON_CONNECT_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_START_HANDSHAKE, RSRX_ACTION_START_SUPERVISION_TIMER, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_INITIALIZED,
		RSRX_EVENT_VALID_INBOUND_CONNECT,
		RSRX_STATE_CONNECTING,
		RSRX_STATUS_OK,
		RSRX_REASON_INBOUND_CONNECT_ACCEPTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_ACCEPT_INBOUND_CONNECT, RSRX_ACTION_START_SUPERVISION_TIMER, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_INITIALIZED,
		RSRX_EVENT_SHUTDOWN_REQUEST,
		RSRX_STATE_SHUTDOWN,
		RSRX_STATUS_OK,
		RSRX_REASON_SHUTDOWN_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_FINALIZE_SHUTDOWN, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_NONE, RSRX_ACTION_NONE },
		2U
	},
	{
		RSRX_STATE_CONNECTING,
		RSRX_EVENT_HANDSHAKE_SUCCESS,
		RSRX_STATE_ESTABLISHED,
		RSRX_STATUS_OK,
		RSRX_REASON_HANDSHAKE_COMPLETED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_RESET_SUPERVISION_TIMER, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_CONNECTING,
		RSRX_EVENT_VALID_HEARTBEAT,
		RSRX_STATE_CONNECTING,
		RSRX_STATUS_OK,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		RSRX_DIAG_INFO_OPERATIONAL_EVENT,
		{ RSRX_ACTION_RESET_SUPERVISION_TIMER, RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NONE, RSRX_ACTION_NONE },
		2U
	},
	{
		RSRX_STATE_CONNECTING,
		RSRX_EVENT_TIMEOUT,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_TIMEOUT_EXPIRED,
		RSRX_DIAG_ERROR_TIMEOUT,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_CONNECTING,
		RSRX_EVENT_INVALID_MESSAGE,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_INVALID_MESSAGE_RECEIVED,
		RSRX_DIAG_ERROR_PROTOCOL,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_CONNECTING,
		RSRX_EVENT_VERSION_MISMATCH,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_VERSION_MISMATCH_DETECTED,
		RSRX_DIAG_ERROR_PROTOCOL,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_CONNECTING,
		RSRX_EVENT_SHUTDOWN_REQUEST,
		RSRX_STATE_SHUTDOWN,
		RSRX_STATUS_OK,
		RSRX_REASON_SHUTDOWN_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_RELEASE_CONNECTION_RESOURCES, RSRX_ACTION_FINALIZE_SHUTDOWN, RSRX_ACTION_NOTIFY_API },
		4U
	},
	{
		RSRX_STATE_ESTABLISHED,
		RSRX_EVENT_VALID_HEARTBEAT,
		RSRX_STATE_ESTABLISHED,
		RSRX_STATUS_OK,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		RSRX_DIAG_INFO_OPERATIONAL_EVENT,
		{ RSRX_ACTION_RESET_SUPERVISION_TIMER, RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NONE, RSRX_ACTION_NONE },
		2U
	},
	{
		RSRX_STATE_ESTABLISHED,
		RSRX_EVENT_VALID_DATA,
		RSRX_STATE_ESTABLISHED,
		RSRX_STATUS_OK,
		RSRX_REASON_DATA_ACCEPTED,
		RSRX_DIAG_INFO_OPERATIONAL_EVENT,
		{ RSRX_ACTION_RESET_SUPERVISION_TIMER, RSRX_ACTION_DELIVER_DATA, RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_ESTABLISHED,
		RSRX_EVENT_SEQUENCE_GAP_DETECTED,
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_STATUS_OK,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_REQUEST_RETRANSMISSION, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_ESTABLISHED,
		RSRX_EVENT_DISCONNECT_REQUEST,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_OK,
		RSRX_REASON_DISCONNECT_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_ESTABLISHED,
		RSRX_EVENT_TIMEOUT,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_TIMEOUT_EXPIRED,
		RSRX_DIAG_ERROR_TIMEOUT,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_ESTABLISHED,
		RSRX_EVENT_PROTOCOL_ERROR,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_PROTOCOL_ERROR_DETECTED,
		RSRX_DIAG_ERROR_PROTOCOL,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_ESTABLISHED,
		RSRX_EVENT_SHUTDOWN_REQUEST,
		RSRX_STATE_SHUTDOWN,
		RSRX_STATUS_OK,
		RSRX_REASON_SHUTDOWN_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_RELEASE_CONNECTION_RESOURCES, RSRX_ACTION_FINALIZE_SHUTDOWN, RSRX_ACTION_NOTIFY_API },
		4U
	},
	{
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_EVENT_RECOVERY_SUCCESS,
		RSRX_STATE_ESTABLISHED,
		RSRX_STATUS_OK,
		RSRX_REASON_RECOVERY_COMPLETED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_CLEAR_RETRANSMISSION_CONTEXT, RSRX_ACTION_RESET_SUPERVISION_TIMER, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_EVENT_VALID_HEARTBEAT,
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_STATUS_OK,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		RSRX_DIAG_INFO_OPERATIONAL_EVENT,
		{ RSRX_ACTION_RESET_SUPERVISION_TIMER, RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NONE, RSRX_ACTION_NONE },
		2U
	},
	{
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_EVENT_RETRANSMISSION_FAILURE,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_RETRANSMISSION_FAILED,
		RSRX_DIAG_ERROR_PROTOCOL,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_EVENT_INVALID_RESPONSE,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_INVALID_RESPONSE_RECEIVED,
		RSRX_DIAG_ERROR_PROTOCOL,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_EVENT_TIMEOUT,
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_STATUS_REJECTED,
		RSRX_REASON_TIMEOUT_EXPIRED,
		RSRX_DIAG_ERROR_TIMEOUT,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_ENTER_FAILSAFE, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		RSRX_STATE_RETRANSMISSION_PENDING,
		RSRX_EVENT_SHUTDOWN_REQUEST,
		RSRX_STATE_SHUTDOWN,
		RSRX_STATUS_OK,
		RSRX_REASON_SHUTDOWN_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_SEND_DISCONNECT, RSRX_ACTION_RELEASE_CONNECTION_RESOURCES, RSRX_ACTION_FINALIZE_SHUTDOWN, RSRX_ACTION_NOTIFY_API },
		4U
	},
	{
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_EVENT_CLEANUP_COMPLETE,
		RSRX_STATE_INITIALIZED,
		RSRX_STATUS_OK,
		RSRX_REASON_CLEANUP_COMPLETED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_RELEASE_CONNECTION_RESOURCES, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_LOG_DIAGNOSTIC, RSRX_ACTION_NONE },
		3U
	},
	{
		RSRX_STATE_SAFE_DISCONNECT,
		RSRX_EVENT_SHUTDOWN_REQUEST,
		RSRX_STATE_SHUTDOWN,
		RSRX_STATUS_OK,
		RSRX_REASON_SHUTDOWN_REQUESTED,
		RSRX_DIAG_INFO_STATE_TRANSITION,
		{ RSRX_ACTION_RELEASE_CONNECTION_RESOURCES, RSRX_ACTION_FINALIZE_SHUTDOWN, RSRX_ACTION_NOTIFY_API, RSRX_ACTION_NONE },
		3U
	}
};

static void vBuildRejectedResult(
	rsrx_transition_result_t * pxResult,
	rsrx_state_t eCurrentState)
{
	pxResult->ePreviousState = eCurrentState;
	pxResult->eNextState = eCurrentState;
	pxResult->eStatus = RSRX_STATUS_REJECTED;
	pxResult->eReason = RSRX_REASON_UNEXPECTED_EVENT_REJECTED;
	pxResult->eDiagnostic = RSRX_DIAG_WARN_REJECTED_EVENT;

	vCopyActions(
		&pxResult->xActions,
		xeRejectActions,
		2U);
}

static void vBuildSafeDisconnectResult(
	rsrx_transition_result_t * pxResult,
	rsrx_state_machine_context_t * pxContext)
{
	pxResult->ePreviousState = pxContext->eCurrentState;
	pxResult->eNextState = RSRX_STATE_SAFE_DISCONNECT;
	pxResult->eStatus = RSRX_STATUS_REJECTED;
	pxResult->eReason = RSRX_REASON_CONSERVATIVE_FAILSAFE;
	pxResult->eDiagnostic = RSRX_DIAG_ERROR_PROTOCOL;

	vCopyActions(
		&pxResult->xActions,
		xeFailSafeActions,
		D_RSRX_ACTION_CAPACITY);

	pxContext->eCurrentState = RSRX_STATE_SAFE_DISCONNECT;
	pxContext->eLastStatus = RSRX_STATUS_REJECTED;
	pxContext->eLastReason = RSRX_REASON_CONSERVATIVE_FAILSAFE;
	pxContext->eLastDiagnostic = RSRX_DIAG_ERROR_PROTOCOL;
}

static void vBuildIgnoredSafeDisconnectResult(
	rsrx_transition_result_t * pxResult,
	rsrx_state_machine_context_t * pxContext)
{
	pxResult->ePreviousState = pxContext->eCurrentState;
	pxResult->eNextState = RSRX_STATE_SAFE_DISCONNECT;
	pxResult->eStatus = RSRX_STATUS_OK;
	pxResult->eReason = RSRX_REASON_IGNORED_IN_SAFE_DISCONNECT;
	pxResult->eDiagnostic = RSRX_DIAG_WARN_IGNORED_EVENT;

	vCopyActions(
		&pxResult->xActions,
		(const rsrx_action_t[])
		{
			RSRX_ACTION_LOG_DIAGNOSTIC
		},
		1U);

	pxContext->eLastStatus = RSRX_STATUS_OK;
	pxContext->eLastReason = RSRX_REASON_IGNORED_IN_SAFE_DISCONNECT;
	pxContext->eLastDiagnostic = RSRX_DIAG_WARN_IGNORED_EVENT;
}

static const rsrx_transition_rule_t * pxFindTransitionRule(
	rsrx_state_t eState,
	rsrx_event_t eEvent)
{
	uint32_t uIndex;

	for(uIndex = 0U; uIndex < (sizeof(xTransitionRules) / sizeof(xTransitionRules[0])); ++uIndex)
	{
		if((xTransitionRules[uIndex].eCurrentState == eState) &&
			(xTransitionRules[uIndex].eEvent == eEvent))
		{
			return &xTransitionRules[uIndex];
		}
	}

	return (const rsrx_transition_rule_t *)0;
}

rsrx_status_t rsrx_state_machine_init(
	rsrx_state_machine_context_t * pxContext)
{
	if(pxContext == (rsrx_state_machine_context_t *)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxContext->eCurrentState = RSRX_STATE_UNINITIALIZED;
	pxContext->eLastStatus = RSRX_STATUS_OK;
	pxContext->eLastReason = RSRX_REASON_NONE;
	pxContext->eLastDiagnostic = RSRX_DIAG_NONE;
	pxContext->uEventCounter = 0U;

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_state_machine_handle_event(
	rsrx_state_machine_context_t * pxContext,
	rsrx_event_t eEvent,
	rsrx_transition_result_t * pxResult)
{
	rsrx_state_t eCurrentState;
	const rsrx_transition_rule_t * pxRule;

	if((pxContext == (rsrx_state_machine_context_t *)0) ||
		(pxResult == (rsrx_transition_result_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	eCurrentState = pxContext->eCurrentState;

	if(uIsStateValid(eCurrentState) == 0U)
	{
		vSetNoActionResult(
			pxResult,
			RSRX_STATE_INVALID,
			RSRX_STATUS_INVALID_STATE,
			RSRX_REASON_INVALID_STATE_VALUE,
			RSRX_DIAG_ERROR_INTERNAL_STATE);
		pxContext->eLastStatus = RSRX_STATUS_INVALID_STATE;
		pxContext->eLastReason = RSRX_REASON_INVALID_STATE_VALUE;
		pxContext->eLastDiagnostic = RSRX_DIAG_ERROR_INTERNAL_STATE;
		return RSRX_STATUS_INVALID_STATE;
	}

	if(uIsEventValid(eEvent) == 0U)
	{
		vSetNoActionResult(
			pxResult,
			eCurrentState,
			RSRX_STATUS_INVALID_EVENT,
			RSRX_REASON_INVALID_EVENT_ENUM,
			RSRX_DIAG_ERROR_INTERFACE);
		pxContext->eLastStatus = RSRX_STATUS_INVALID_EVENT;
		pxContext->eLastReason = RSRX_REASON_INVALID_EVENT_ENUM;
		pxContext->eLastDiagnostic = RSRX_DIAG_ERROR_INTERFACE;
		return RSRX_STATUS_INVALID_EVENT;
	}

	pxContext->uEventCounter++;

	if(eCurrentState == RSRX_STATE_SHUTDOWN)
	{
		vSetNoActionResult(
			pxResult,
			RSRX_STATE_SHUTDOWN,
			RSRX_STATUS_OK,
			RSRX_REASON_IGNORED_IN_SHUTDOWN,
			RSRX_DIAG_WARN_IGNORED_EVENT);
		pxContext->eLastStatus = RSRX_STATUS_OK;
		pxContext->eLastReason = RSRX_REASON_IGNORED_IN_SHUTDOWN;
		pxContext->eLastDiagnostic = RSRX_DIAG_WARN_IGNORED_EVENT;
		return RSRX_STATUS_OK;
	}

	pxRule = pxFindTransitionRule(eCurrentState, eEvent);

	if(pxRule != (const rsrx_transition_rule_t *)0)
	{
		vApplyTransitionRule(pxResult, pxContext, pxRule);
		return pxRule->eStatus;
	}

	if(uStateUsesConservativeFailSafePolicy(eCurrentState) != 0U)
	{
		vBuildSafeDisconnectResult(pxResult, pxContext);
		return RSRX_STATUS_REJECTED;
	}

	if(eCurrentState == RSRX_STATE_SAFE_DISCONNECT)
	{
		vBuildIgnoredSafeDisconnectResult(pxResult, pxContext);
		return RSRX_STATUS_OK;
	}

	vBuildRejectedResult(pxResult, eCurrentState);
	pxContext->eLastStatus = RSRX_STATUS_REJECTED;
	pxContext->eLastReason = RSRX_REASON_UNEXPECTED_EVENT_REJECTED;
	pxContext->eLastDiagnostic = RSRX_DIAG_WARN_REJECTED_EVENT;

	return RSRX_STATUS_REJECTED;
}

rsrx_state_t rsrx_state_machine_get_state(
	const rsrx_state_machine_context_t * pxContext)
{
	if(pxContext == (const rsrx_state_machine_context_t *)0)
	{
		return RSRX_STATE_INVALID;
	}

	return pxContext->eCurrentState;
}

rsrx_status_t rsrx_state_machine_reset(
	rsrx_state_machine_context_t * pxContext)
{
	if(pxContext == (rsrx_state_machine_context_t *)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxContext->eCurrentState = RSRX_STATE_UNINITIALIZED;
	pxContext->eLastStatus = RSRX_STATUS_OK;
	pxContext->eLastReason = RSRX_REASON_NONE;
	pxContext->eLastDiagnostic = RSRX_DIAG_NONE;
	pxContext->uEventCounter = 0U;

	return RSRX_STATUS_OK;
}
