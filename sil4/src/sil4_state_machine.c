#include "sil4_state_machine.h"

typedef struct
{
	sil4_state_t eCurrentState;
	sil4_event_t eEvent;
	sil4_state_t eNextState;
	sil4_status_t eStatus;
	sil4_action_t eActions[D_SIL4_ACTION_CAPACITY];
	uint32_t uActionCount;
} sil4_transition_rule_t;

static const sil4_action_t xeRejectActions[2] =
{
	SIL4_ACTION_LOG_DIAGNOSTIC,
	SIL4_ACTION_NOTIFY_API
};

static const sil4_action_t xeFailSafeActions[D_SIL4_ACTION_CAPACITY] =
{
	SIL4_ACTION_SEND_DISCONNECT,
	SIL4_ACTION_ENTER_FAILSAFE,
	SIL4_ACTION_NOTIFY_API,
	SIL4_ACTION_LOG_DIAGNOSTIC
};

static void vSetNoActionResult(
	sil4_transition_result_t * pxResult,
	sil4_state_t eCurrentState,
	sil4_status_t eStatus)
{
	uint32_t uIndex;

	pxResult->ePreviousState = eCurrentState;
	pxResult->eNextState = eCurrentState;
	pxResult->eStatus = eStatus;
	pxResult->xActions.uActionCount = 0U;

	for(uIndex = 0U; uIndex < D_SIL4_ACTION_CAPACITY; ++uIndex)
	{
		pxResult->xActions.eActions[uIndex] = SIL4_ACTION_NONE;
	}
}

static void vCopyActions(
	sil4_action_list_t * pxDestination,
	const sil4_action_t * peSourceActions,
	uint32_t uActionCount)
{
	uint32_t uIndex;

	pxDestination->uActionCount = uActionCount;

	for(uIndex = 0U; uIndex < D_SIL4_ACTION_CAPACITY; ++uIndex)
	{
		if(uIndex < uActionCount)
		{
			pxDestination->eActions[uIndex] = peSourceActions[uIndex];
		}
		else
		{
			pxDestination->eActions[uIndex] = SIL4_ACTION_NONE;
		}
	}
}

static void vApplyTransitionRule(
	sil4_transition_result_t * pxResult,
	sil4_state_machine_context_t * pxContext,
	const sil4_transition_rule_t * pxRule)
{
	pxResult->ePreviousState = pxContext->eCurrentState;
	pxResult->eNextState = pxRule->eNextState;
	pxResult->eStatus = pxRule->eStatus;

	vCopyActions(
		&pxResult->xActions,
		pxRule->eActions,
		pxRule->uActionCount);

	pxContext->eCurrentState = pxRule->eNextState;
	pxContext->eLastStatus = pxRule->eStatus;
}

static uint32_t uIsStateValid(
	sil4_state_t eState)
{
	return (uint32_t)((eState >= SIL4_STATE_UNINITIALIZED) &&
		(eState < SIL4_STATE_INVALID));
}

static uint32_t uIsEventValid(
	sil4_event_t eEvent)
{
	return (uint32_t)((eEvent >= SIL4_EVENT_INIT_SUCCESS) &&
		(eEvent < SIL4_EVENT_INVALID));
}

static uint32_t uStateUsesConservativeFailSafePolicy(
	sil4_state_t eState)
{
	return (uint32_t)((eState == SIL4_STATE_CONNECTING) ||
		(eState == SIL4_STATE_ESTABLISHED) ||
		(eState == SIL4_STATE_RETRANSMISSION_PENDING));
}

static const sil4_transition_rule_t * pxFindTransitionRule(
	sil4_state_t eState,
	sil4_event_t eEvent);

static void vBuildSafeDisconnectResult(
	sil4_transition_result_t * pxResult,
	sil4_state_machine_context_t * pxContext);

static void vBuildIgnoredSafeDisconnectResult(
	sil4_transition_result_t * pxResult,
	sil4_state_machine_context_t * pxContext);

static const sil4_transition_rule_t xTransitionRules[] =
{
	{
		SIL4_STATE_UNINITIALIZED,
		SIL4_EVENT_INIT_SUCCESS,
		SIL4_STATE_INITIALIZED,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_NONE, SIL4_ACTION_NONE },
		2U
	},
	{
		SIL4_STATE_UNINITIALIZED,
		SIL4_EVENT_INIT_FAILURE,
		SIL4_STATE_SHUTDOWN,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_FINALIZE_SHUTDOWN, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_UNINITIALIZED,
		SIL4_EVENT_SHUTDOWN_REQUEST,
		SIL4_STATE_SHUTDOWN,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_FINALIZE_SHUTDOWN, SIL4_ACTION_NONE, SIL4_ACTION_NONE, SIL4_ACTION_NONE },
		1U
	},
	{
		SIL4_STATE_INITIALIZED,
		SIL4_EVENT_CONNECT_REQUEST,
		SIL4_STATE_CONNECTING,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_START_HANDSHAKE, SIL4_ACTION_START_SUPERVISION_TIMER, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_INITIALIZED,
		SIL4_EVENT_VALID_INBOUND_CONNECT,
		SIL4_STATE_CONNECTING,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_ACCEPT_INBOUND_CONNECT, SIL4_ACTION_START_SUPERVISION_TIMER, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_INITIALIZED,
		SIL4_EVENT_SHUTDOWN_REQUEST,
		SIL4_STATE_SHUTDOWN,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_FINALIZE_SHUTDOWN, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_NONE, SIL4_ACTION_NONE },
		2U
	},
	{
		SIL4_STATE_CONNECTING,
		SIL4_EVENT_HANDSHAKE_SUCCESS,
		SIL4_STATE_ESTABLISHED,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_RESET_SUPERVISION_TIMER, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_CONNECTING,
		SIL4_EVENT_VALID_HEARTBEAT,
		SIL4_STATE_CONNECTING,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_RESET_SUPERVISION_TIMER, SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NONE, SIL4_ACTION_NONE },
		2U
	},
	{
		SIL4_STATE_CONNECTING,
		SIL4_EVENT_TIMEOUT,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_CONNECTING,
		SIL4_EVENT_INVALID_MESSAGE,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_CONNECTING,
		SIL4_EVENT_VERSION_MISMATCH,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_CONNECTING,
		SIL4_EVENT_SHUTDOWN_REQUEST,
		SIL4_STATE_SHUTDOWN,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_RELEASE_CONNECTION_RESOURCES, SIL4_ACTION_FINALIZE_SHUTDOWN, SIL4_ACTION_NOTIFY_API },
		4U
	},
	{
		SIL4_STATE_ESTABLISHED,
		SIL4_EVENT_VALID_HEARTBEAT,
		SIL4_STATE_ESTABLISHED,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_RESET_SUPERVISION_TIMER, SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NONE, SIL4_ACTION_NONE },
		2U
	},
	{
		SIL4_STATE_ESTABLISHED,
		SIL4_EVENT_VALID_DATA,
		SIL4_STATE_ESTABLISHED,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_RESET_SUPERVISION_TIMER, SIL4_ACTION_DELIVER_DATA, SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_ESTABLISHED,
		SIL4_EVENT_SEQUENCE_GAP_DETECTED,
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_REQUEST_RETRANSMISSION, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_ESTABLISHED,
		SIL4_EVENT_DISCONNECT_REQUEST,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_ESTABLISHED,
		SIL4_EVENT_TIMEOUT,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_ESTABLISHED,
		SIL4_EVENT_PROTOCOL_ERROR,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_ESTABLISHED,
		SIL4_EVENT_SHUTDOWN_REQUEST,
		SIL4_STATE_SHUTDOWN,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_RELEASE_CONNECTION_RESOURCES, SIL4_ACTION_FINALIZE_SHUTDOWN, SIL4_ACTION_NOTIFY_API },
		4U
	},
	{
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_EVENT_RECOVERY_SUCCESS,
		SIL4_STATE_ESTABLISHED,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_CLEAR_RETRANSMISSION_CONTEXT, SIL4_ACTION_RESET_SUPERVISION_TIMER, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_EVENT_VALID_HEARTBEAT,
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_RESET_SUPERVISION_TIMER, SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NONE, SIL4_ACTION_NONE },
		2U
	},
	{
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_EVENT_RETRANSMISSION_FAILURE,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_EVENT_INVALID_RESPONSE,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_EVENT_TIMEOUT,
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_STATUS_REJECTED,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_ENTER_FAILSAFE, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC },
		4U
	},
	{
		SIL4_STATE_RETRANSMISSION_PENDING,
		SIL4_EVENT_SHUTDOWN_REQUEST,
		SIL4_STATE_SHUTDOWN,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_SEND_DISCONNECT, SIL4_ACTION_RELEASE_CONNECTION_RESOURCES, SIL4_ACTION_FINALIZE_SHUTDOWN, SIL4_ACTION_NOTIFY_API },
		4U
	},
	{
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_EVENT_CLEANUP_COMPLETE,
		SIL4_STATE_INITIALIZED,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_RELEASE_CONNECTION_RESOURCES, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_LOG_DIAGNOSTIC, SIL4_ACTION_NONE },
		3U
	},
	{
		SIL4_STATE_SAFE_DISCONNECT,
		SIL4_EVENT_SHUTDOWN_REQUEST,
		SIL4_STATE_SHUTDOWN,
		SIL4_STATUS_OK,
		{ SIL4_ACTION_RELEASE_CONNECTION_RESOURCES, SIL4_ACTION_FINALIZE_SHUTDOWN, SIL4_ACTION_NOTIFY_API, SIL4_ACTION_NONE },
		3U
	}
};

static void vBuildRejectedResult(
	sil4_transition_result_t * pxResult,
	sil4_state_t eCurrentState)
{
	pxResult->ePreviousState = eCurrentState;
	pxResult->eNextState = eCurrentState;
	pxResult->eStatus = SIL4_STATUS_REJECTED;

	vCopyActions(
		&pxResult->xActions,
		xeRejectActions,
		2U);
}

static void vBuildSafeDisconnectResult(
	sil4_transition_result_t * pxResult,
	sil4_state_machine_context_t * pxContext)
{
	pxResult->ePreviousState = pxContext->eCurrentState;
	pxResult->eNextState = SIL4_STATE_SAFE_DISCONNECT;
	pxResult->eStatus = SIL4_STATUS_REJECTED;

	vCopyActions(
		&pxResult->xActions,
		xeFailSafeActions,
		D_SIL4_ACTION_CAPACITY);

	pxContext->eCurrentState = SIL4_STATE_SAFE_DISCONNECT;
	pxContext->eLastStatus = SIL4_STATUS_REJECTED;
}

static void vBuildIgnoredSafeDisconnectResult(
	sil4_transition_result_t * pxResult,
	sil4_state_machine_context_t * pxContext)
{
	pxResult->ePreviousState = pxContext->eCurrentState;
	pxResult->eNextState = SIL4_STATE_SAFE_DISCONNECT;
	pxResult->eStatus = SIL4_STATUS_OK;

	vCopyActions(
		&pxResult->xActions,
		(const sil4_action_t[])
		{
			SIL4_ACTION_LOG_DIAGNOSTIC
		},
		1U);

	pxContext->eLastStatus = SIL4_STATUS_OK;
}

static const sil4_transition_rule_t * pxFindTransitionRule(
	sil4_state_t eState,
	sil4_event_t eEvent)
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

	return (const sil4_transition_rule_t *)0;
}

sil4_status_t state_machine_init(
	sil4_state_machine_context_t * pxContext)
{
	if(pxContext == (sil4_state_machine_context_t *)0)
	{
		return SIL4_STATUS_INVALID_ARGUMENT;
	}

	pxContext->eCurrentState = SIL4_STATE_UNINITIALIZED;
	pxContext->eLastStatus = SIL4_STATUS_OK;
	pxContext->uEventCounter = 0U;

	return SIL4_STATUS_OK;
}

sil4_status_t state_machine_handle_event(
	sil4_state_machine_context_t * pxContext,
	sil4_event_t eEvent,
	sil4_transition_result_t * pxResult)
{
	sil4_state_t eCurrentState;
	const sil4_transition_rule_t * pxRule;

	if((pxContext == (sil4_state_machine_context_t *)0) ||
		(pxResult == (sil4_transition_result_t *)0))
	{
		return SIL4_STATUS_INVALID_ARGUMENT;
	}

	eCurrentState = pxContext->eCurrentState;

	if(uIsStateValid(eCurrentState) == 0U)
	{
		vSetNoActionResult(pxResult, SIL4_STATE_INVALID, SIL4_STATUS_INVALID_STATE);
		pxContext->eLastStatus = SIL4_STATUS_INVALID_STATE;
		return SIL4_STATUS_INVALID_STATE;
	}

	if(uIsEventValid(eEvent) == 0U)
	{
		vSetNoActionResult(pxResult, eCurrentState, SIL4_STATUS_INVALID_EVENT);
		pxContext->eLastStatus = SIL4_STATUS_INVALID_EVENT;
		return SIL4_STATUS_INVALID_EVENT;
	}

	pxContext->uEventCounter++;

	if(eCurrentState == SIL4_STATE_SHUTDOWN)
	{
		vSetNoActionResult(pxResult, SIL4_STATE_SHUTDOWN, SIL4_STATUS_OK);
		pxContext->eLastStatus = SIL4_STATUS_OK;
		return SIL4_STATUS_OK;
	}

	pxRule = pxFindTransitionRule(eCurrentState, eEvent);

	if(pxRule != (const sil4_transition_rule_t *)0)
	{
		vApplyTransitionRule(pxResult, pxContext, pxRule);
		return pxRule->eStatus;
	}

	if(uStateUsesConservativeFailSafePolicy(eCurrentState) != 0U)
	{
		vBuildSafeDisconnectResult(pxResult, pxContext);
		return SIL4_STATUS_REJECTED;
	}

	if(eCurrentState == SIL4_STATE_SAFE_DISCONNECT)
	{
		vBuildIgnoredSafeDisconnectResult(pxResult, pxContext);
		return SIL4_STATUS_OK;
	}

	vBuildRejectedResult(pxResult, eCurrentState);
	pxContext->eLastStatus = SIL4_STATUS_REJECTED;

	return SIL4_STATUS_REJECTED;
}

sil4_state_t state_machine_get_state(
	const sil4_state_machine_context_t * pxContext)
{
	if(pxContext == (const sil4_state_machine_context_t *)0)
	{
		return SIL4_STATE_INVALID;
	}

	return pxContext->eCurrentState;
}

sil4_status_t state_machine_reset(
	sil4_state_machine_context_t * pxContext)
{
	if(pxContext == (sil4_state_machine_context_t *)0)
	{
		return SIL4_STATUS_INVALID_ARGUMENT;
	}

	pxContext->eCurrentState = SIL4_STATE_UNINITIALIZED;
	pxContext->eLastStatus = SIL4_STATUS_OK;
	pxContext->uEventCounter = 0U;

	return SIL4_STATUS_OK;
}
