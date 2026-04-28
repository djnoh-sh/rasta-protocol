#include "rsrx_transport_supervisor.h"

#define D_RSRX_SUPERVISOR_DEFAULT_SEND_FAILURE_BUDGET (2U)
#define D_RSRX_SUPERVISOR_DEFAULT_RECEIVE_ERROR_BUDGET (2U)

static uint32_t uCountAvailableChannels(
	const rsrx_transport_supervisor_context_t * pxContext);

static uint32_t uGetEffectiveHoldoffTarget(
	const rsrx_channel_manager_context_t * pxChannelManager)
{
	uint32_t uTarget;

	uTarget = pxChannelManager->xConfig.uPreferredRecoveryHoldoffSelections;
	if((UINT32_MAX - uTarget) < pxChannelManager->uPreferredRecoveryPendingPenaltySelections)
	{
		return UINT32_MAX;
	}

	return uTarget + pxChannelManager->uPreferredRecoveryPendingPenaltySelections;
}

static void vResetSupervisorReport(
	rsrx_transport_supervisor_report_t * pxReport)
{
	pxReport->xLastChannelState.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->xLastChannelState.uIsAvailable = 0U;
	pxReport->xLastFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->xLastFrame.puPayload = (const uint8_t *)0;
	pxReport->xLastFrame.xPayloadLength = 0U;
	pxReport->xLastFrame.eEventType = RSRX_TRANSPORT_EVENT_NONE;
	pxReport->xLastMessage.eMessageType = RSRX_MESSAGE_TYPE_INVALID;
	pxReport->xLastMessage.eSuggestedEvent = RSRX_EVENT_INVALID;
	pxReport->xLastMessage.eReason = RSRX_REASON_NONE;
	pxReport->xLastMessage.uSequenceNumber = 0U;
	pxReport->xLastMessage.uConfirmationNumber = 0U;
	pxReport->xLastMessage.xPayloadLength = 0U;
	pxReport->eLastEffectiveEvent = RSRX_EVENT_INVALID;
	pxReport->eLastSessionStatus = RSRX_STATUS_OK;
	pxReport->eLastDecision = RSRX_SUPERVISOR_DECISION_NONE;
	pxReport->eLastDecisionClass = RSRX_SUPERVISOR_DECISION_CLASS_NONE;
	pxReport->eLastBudgetUpdate = RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
	pxReport->eLastReceiveErrorStage = RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_NONE;
	pxReport->eLastReceiveTransportStatus = RSRX_TRANSPORT_STATUS_OK;
	pxReport->eBudgetChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->pxLastReport = (const rsrx_orchestrator_report_t *)0;
	pxReport->uProcessedFrameCount = 0U;
	pxReport->uPollCount = 0U;
	pxReport->uConsecutiveSendFailureCount = 0U;
	pxReport->uSendFailureBudgetResetCount = 0U;
	pxReport->uConsecutiveReceiveErrorCount = 0U;
	pxReport->uReceiveErrorBudgetResetCount = 0U;
	pxReport->uAcceptedDecisionCount = 0U;
	pxReport->uRejectedDecisionCount = 0U;
	pxReport->uIgnoredDecisionCount = 0U;
	pxReport->uErrorDecisionCount = 0U;
	pxReport->uChannelSwitchCount = 0U;
	pxReport->uLastChannelSwitchOccurred = 0U;
	pxReport->uAvailableChannelCount = 0U;
	pxReport->uChannelUnavailableSelectionCount = 0U;
	pxReport->uFailoverSwitchCount = 0U;
	pxReport->uPreferredRecoverySwitchCount = 0U;
	pxReport->uImmediatePreferredRecoverySwitchCount = 0U;
	pxReport->uHoldoffPreferredRecoverySwitchCount = 0U;
	pxReport->uCompletedHoldoffPreferredRecoverySwitchCount = 0U;
	pxReport->uBypassPreferredRecoverySwitchCount = 0U;
	pxReport->uNoOpRefreshCount = 0U;
	pxReport->uPreferredChannelTriggeredRefreshEventCount = 0U;
	pxReport->uNonPreferredChannelTriggeredRefreshEventCount = 0U;
	pxReport->uPreferredChannelTriggeredSwitchCount = 0U;
	pxReport->uNonPreferredChannelTriggeredSwitchCount = 0U;
	pxReport->uPreferredChannelTriggeredNoOpRefreshCount = 0U;
	pxReport->uNonPreferredChannelTriggeredNoOpRefreshCount = 0U;
	pxReport->uHoldoffRefreshNoOpCount = 0U;
	pxReport->uActiveRefreshNoOpCount = 0U;
	pxReport->uHoldoffCycleCount = 0U;
	pxReport->uCompletedHoldoffCycleCount = 0U;
	pxReport->uOrdinaryCompletedHoldoffCycleCount = 0U;
	pxReport->uBypassCompletedHoldoffCycleCount = 0U;
	pxReport->uAbortedHoldoffCycleCount = 0U;
	pxReport->uTerminalHoldoffOutcomeCount = 0U;
	pxReport->uOrdinaryTerminalHoldoffOutcomeCount = 0U;
	pxReport->uBypassTerminalHoldoffOutcomeCount = 0U;
	pxReport->uAbortedTerminalHoldoffOutcomeCount = 0U;
	pxReport->uPreferredChannelTriggeredTerminalHoldoffOutcomeCount = 0U;
	pxReport->uNonPreferredChannelTriggeredTerminalHoldoffOutcomeCount = 0U;
	pxReport->uChannelUpTriggeredTerminalHoldoffOutcomeCount = 0U;
	pxReport->uChannelDownTriggeredTerminalHoldoffOutcomeCount = 0U;
	pxReport->uHoldoffResetCount = 0U;
	pxReport->uPreferredRecoveryHoldoffActive = 0U;
	pxReport->uPreferredRecoveryHoldoffProgressCount = 0U;
	pxReport->uPreferredRecoveryPendingPenaltyCount = 0U;
	pxReport->uPreferredRecoveryPenaltyArmCount = 0U;
	pxReport->uPreferredRecoveryPenaltyClearCount = 0U;
	pxReport->uPreferredRecoveryPenaltyBypassClearCount = 0U;
	pxReport->uPreferredRecoveryHoldoffTargetCount = 0U;
	pxReport->uPreferredRecoveryHoldoffRemainingCount = 0U;
	pxReport->eLastHoldoffCycleState = RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_NONE;
	pxReport->eLastHoldoffCycleStartTriggerEventType = RSRX_TRANSPORT_EVENT_NONE;
	pxReport->eLastHoldoffCycleStartTriggerChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->eLastCompletedHoldoffCycleKind =
		RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_NONE;
	pxReport->eLastTerminalHoldoffOutcome =
		RSRX_SUPERVISOR_TERMINAL_HOLDOFF_OUTCOME_NONE;
	pxReport->eLastTerminalHoldoffOutcomeTriggerEventType = RSRX_TRANSPORT_EVENT_NONE;
	pxReport->eLastTerminalHoldoffOutcomeTriggerChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->eLastSwitchKind = RSRX_SUPERVISOR_SWITCH_KIND_NONE;
	pxReport->eLastSwitchReason = RSRX_SUPERVISOR_SWITCH_REASON_NONE;
	pxReport->eLastSwitchTriggerEventType = RSRX_TRANSPORT_EVENT_NONE;
	pxReport->eLastSwitchTriggerChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->eLastSwitchFromChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->eLastSwitchToChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxReport->uLastPumpIterationCount = 0U;
	pxReport->uLastPumpProcessedFrameCount = 0U;
	pxReport->uOutstandingSendPresent = 0U;
	pxReport->uDeferredSendPresent = 0U;
	pxReport->uDeferredSendCount = 0U;
	pxReport->uQueuedSendCount = 0U;
	pxReport->uMaxDeferredSendCount = 0U;
	pxReport->uDeferredDispatchCount = 0U;
	pxReport->uQueueOverflowRejectCount = 0U;
	pxReport->eLastOutboundRejectReason = RSRX_OUTBOUND_REJECT_REASON_NONE;
	pxReport->uBusyRejectedSendCount = 0U;
	pxReport->uConsecutiveBusyRejectedSendCount = 0U;
	pxReport->uMaxConsecutiveBusyRejectedSendCount = 0U;
	pxReport->uBusyRejectEscalationCount = 0U;
	pxReport->uLastBusyRejectEscalated = 0U;
}

static rsrx_supervisor_decision_class_t eMapDecisionClass(
	rsrx_supervisor_decision_t eDecision)
{
	switch(eDecision)
	{
		case RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED:
		case RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED:
			return RSRX_SUPERVISOR_DECISION_CLASS_ACCEPTED;

		case RSRX_SUPERVISOR_DECISION_SESSION_REJECTED:
			return RSRX_SUPERVISOR_DECISION_CLASS_REJECTED;

		case RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE:
		case RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED:
		case RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED:
		case RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED:
		case RSRX_SUPERVISOR_DECISION_SEND_FAILURE_INACTIVE_CHANNEL_IGNORED:
		case RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED:
		case RSRX_SUPERVISOR_DECISION_TRANSPORT_EVENT_IGNORED:
			return RSRX_SUPERVISOR_DECISION_CLASS_IGNORED;

		case RSRX_SUPERVISOR_DECISION_DECODE_FAILED:
		case RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN:
		case RSRX_SUPERVISOR_DECISION_SEND_FAILURE_ESCALATED:
		case RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_ESCALATED:
			return RSRX_SUPERVISOR_DECISION_CLASS_ERROR;

		case RSRX_SUPERVISOR_DECISION_NONE:
		default:
			return RSRX_SUPERVISOR_DECISION_CLASS_NONE;
	}
}

static void vRecordDecision(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_supervisor_decision_t eDecision)
{
	rsrx_supervisor_decision_class_t eDecisionClass;

	pxContext->xLastReport.eLastDecision = eDecision;
	eDecisionClass = eMapDecisionClass(eDecision);
	pxContext->xLastReport.eLastDecisionClass = eDecisionClass;

	switch(eDecisionClass)
	{
		case RSRX_SUPERVISOR_DECISION_CLASS_ACCEPTED:
			if(pxContext->xLastReport.uAcceptedDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uAcceptedDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_REJECTED:
			if(pxContext->xLastReport.uRejectedDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uRejectedDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_IGNORED:
			if(pxContext->xLastReport.uIgnoredDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uIgnoredDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_ERROR:
			if(pxContext->xLastReport.uErrorDecisionCount < UINT32_MAX)
			{
				pxContext->xLastReport.uErrorDecisionCount++;
			}
			break;

		case RSRX_SUPERVISOR_DECISION_CLASS_NONE:
		default:
			break;
	}
}

static void vRefreshChannelSwitchTelemetry(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t ePreviousActiveChannelId,
	rsrx_transport_event_type_t eTriggerEventType,
	rsrx_transport_channel_id_t eTriggerChannelId)
{
	rsrx_transport_channel_id_t eCurrentActiveChannelId;
	rsrx_transport_channel_id_t ePreferredChannelId;
	uint32_t uPreviousHoldoffProgressCount;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (rsrx_session_t *)0))
	{
		return;
	}

	eCurrentActiveChannelId = rsrx_channel_manager_get_active_channel(
		&pxContext->pxSession->xChannelManager);
	uPreviousHoldoffProgressCount =
		pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount;
	ePreferredChannelId =
		pxContext->pxSession->xChannelManager.xConfig.axChannels[
			pxContext->pxSession->xChannelManager.xConfig.uPreferredChannelIndex].eChannelId;
	pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount =
		pxContext->pxSession->xChannelManager.uPreferredRecoveryStableSelectionCount;
	pxContext->xLastReport.uPreferredRecoveryPendingPenaltyCount =
		pxContext->pxSession->xChannelManager.uPreferredRecoveryPendingPenaltySelections;
	pxContext->xLastReport.uPreferredRecoveryPenaltyArmCount =
		pxContext->pxSession->xChannelManager.uPreferredRecoveryPenaltyArmCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyClearCount =
		pxContext->pxSession->xChannelManager.uPreferredRecoveryPenaltyClearCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyBypassClearCount =
		pxContext->pxSession->xChannelManager.uPreferredRecoveryPenaltyBypassClearCount;
	pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount =
		uGetEffectiveHoldoffTarget(&pxContext->pxSession->xChannelManager);
	pxContext->xLastReport.uPreferredRecoveryHoldoffRemainingCount =
		(pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount >
			pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount) ?
			(pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount -
				pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount) :
			0U;
	pxContext->xLastReport.uAvailableChannelCount = uCountAvailableChannels(pxContext);
	pxContext->xLastReport.uPreferredRecoveryHoldoffActive = (uint32_t)(
		(pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount > 0U) &&
		(pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount <
			pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount) &&
		(eCurrentActiveChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(ePreferredChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(eCurrentActiveChannelId != ePreferredChannelId) &&
		(pxContext->xLastReport.uAvailableChannelCount > 1U));
	pxContext->xLastReport.uChannelSwitchCount =
		pxContext->pxSession->xChannelManager.uTotalSwitchCount;
	pxContext->xLastReport.uLastChannelSwitchOccurred =
		pxContext->pxSession->xChannelManager.uLastSelectionWasFailover;
	pxContext->xLastReport.uChannelUnavailableSelectionCount =
		pxContext->pxSession->xChannelManager.uUnavailableSelectionCount;
	pxContext->xLastReport.eLastSwitchTriggerEventType = eTriggerEventType;
	pxContext->xLastReport.eLastSwitchTriggerChannelId = eTriggerChannelId;
	if(eTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_UP)
	{
		if(eTriggerChannelId == ePreferredChannelId)
		{
			if(pxContext->xLastReport.uPreferredChannelTriggeredRefreshEventCount < UINT32_MAX)
			{
				pxContext->xLastReport.uPreferredChannelTriggeredRefreshEventCount++;
			}
		}
		else if(pxContext->xLastReport.uNonPreferredChannelTriggeredRefreshEventCount < UINT32_MAX)
		{
			pxContext->xLastReport.uNonPreferredChannelTriggeredRefreshEventCount++;
		}
	}
	if(pxContext->xLastReport.uLastChannelSwitchOccurred != 0U)
	{
		if(eTriggerChannelId == ePreferredChannelId)
		{
			if(pxContext->xLastReport.uPreferredChannelTriggeredSwitchCount < UINT32_MAX)
			{
				pxContext->xLastReport.uPreferredChannelTriggeredSwitchCount++;
			}
		}
		else if(pxContext->xLastReport.uNonPreferredChannelTriggeredSwitchCount < UINT32_MAX)
		{
			pxContext->xLastReport.uNonPreferredChannelTriggeredSwitchCount++;
		}
		if(eCurrentActiveChannelId == ePreferredChannelId)
		{
			pxContext->xLastReport.eLastSwitchKind =
				RSRX_SUPERVISOR_SWITCH_KIND_PREFERRED_RECOVERY;
			if(pxContext->xLastReport.uPreferredRecoverySwitchCount < UINT32_MAX)
			{
				pxContext->xLastReport.uPreferredRecoverySwitchCount++;
			}
			if(uPreviousHoldoffProgressCount > 0U)
			{
				if((eTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_DOWN) &&
					(eTriggerChannelId != ePreferredChannelId))
				{
					pxContext->xLastReport.eLastSwitchReason =
						RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_BYPASS_ACTIVE_LOSS;
					if(pxContext->xLastReport.uBypassPreferredRecoverySwitchCount < UINT32_MAX)
					{
						pxContext->xLastReport.uBypassPreferredRecoverySwitchCount++;
					}
					if(pxContext->xLastReport.uBypassCompletedHoldoffCycleCount < UINT32_MAX)
					{
						pxContext->xLastReport.uBypassCompletedHoldoffCycleCount++;
					}
					pxContext->xLastReport.eLastCompletedHoldoffCycleKind =
						RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_BYPASS;
					pxContext->xLastReport.eLastTerminalHoldoffOutcome =
						RSRX_SUPERVISOR_TERMINAL_HOLDOFF_OUTCOME_BYPASS_COMPLETED;
					if(pxContext->xLastReport.uTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uTerminalHoldoffOutcomeCount++;
					}
					if(pxContext->xLastReport.uBypassTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uBypassTerminalHoldoffOutcomeCount++;
					}
					if(pxContext->xLastReport.uNonPreferredChannelTriggeredTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uNonPreferredChannelTriggeredTerminalHoldoffOutcomeCount++;
					}
					if(pxContext->xLastReport.uChannelDownTriggeredTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uChannelDownTriggeredTerminalHoldoffOutcomeCount++;
					}
					pxContext->xLastReport.eLastTerminalHoldoffOutcomeTriggerEventType =
						eTriggerEventType;
					pxContext->xLastReport.eLastTerminalHoldoffOutcomeTriggerChannelId =
						eTriggerChannelId;
				}
				else
				{
					pxContext->xLastReport.eLastSwitchReason =
						RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_AFTER_HOLDOFF;
					if(pxContext->xLastReport.uCompletedHoldoffPreferredRecoverySwitchCount < UINT32_MAX)
					{
						pxContext->xLastReport.uCompletedHoldoffPreferredRecoverySwitchCount++;
					}
					if(pxContext->xLastReport.uOrdinaryCompletedHoldoffCycleCount < UINT32_MAX)
					{
						pxContext->xLastReport.uOrdinaryCompletedHoldoffCycleCount++;
					}
					pxContext->xLastReport.eLastCompletedHoldoffCycleKind =
						RSRX_SUPERVISOR_COMPLETED_HOLDOFF_CYCLE_KIND_ORDINARY;
					pxContext->xLastReport.eLastTerminalHoldoffOutcome =
						RSRX_SUPERVISOR_TERMINAL_HOLDOFF_OUTCOME_ORDINARY_COMPLETED;
					if(pxContext->xLastReport.uTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uTerminalHoldoffOutcomeCount++;
					}
					if(pxContext->xLastReport.uOrdinaryTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uOrdinaryTerminalHoldoffOutcomeCount++;
					}
					if(pxContext->xLastReport.uPreferredChannelTriggeredTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uPreferredChannelTriggeredTerminalHoldoffOutcomeCount++;
					}
					if(pxContext->xLastReport.uChannelUpTriggeredTerminalHoldoffOutcomeCount < UINT32_MAX)
					{
						pxContext->xLastReport.uChannelUpTriggeredTerminalHoldoffOutcomeCount++;
					}
					pxContext->xLastReport.eLastTerminalHoldoffOutcomeTriggerEventType =
						eTriggerEventType;
					pxContext->xLastReport.eLastTerminalHoldoffOutcomeTriggerChannelId =
						eTriggerChannelId;
				}
				if(pxContext->xLastReport.uHoldoffPreferredRecoverySwitchCount < UINT32_MAX)
				{
					pxContext->xLastReport.uHoldoffPreferredRecoverySwitchCount++;
				}
				if(pxContext->xLastReport.uCompletedHoldoffCycleCount < UINT32_MAX)
				{
					pxContext->xLastReport.uCompletedHoldoffCycleCount++;
				}
				pxContext->xLastReport.eLastHoldoffCycleState =
					RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_COMPLETED;
			}
			else
			{
				pxContext->xLastReport.eLastSwitchReason =
					RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_IMMEDIATE;
				if(pxContext->xLastReport.uImmediatePreferredRecoverySwitchCount < UINT32_MAX)
				{
					pxContext->xLastReport.uImmediatePreferredRecoverySwitchCount++;
				}
			}
		}
		else
		{
			pxContext->xLastReport.eLastSwitchKind =
				RSRX_SUPERVISOR_SWITCH_KIND_FAILOVER;
			pxContext->xLastReport.eLastSwitchReason =
				RSRX_SUPERVISOR_SWITCH_REASON_FAILOVER_CHANNEL_DOWN;
			if(pxContext->xLastReport.uFailoverSwitchCount < UINT32_MAX)
			{
				pxContext->xLastReport.uFailoverSwitchCount++;
			}
		}
		pxContext->xLastReport.eLastSwitchFromChannelId = ePreviousActiveChannelId;
		pxContext->xLastReport.eLastSwitchToChannelId = eCurrentActiveChannelId;
	}
	else
	{
		if((pxContext->xLastReport.eLastDecision ==
				RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED) &&
			(pxContext->uNoOpAuditCountedInCurrentCall == 0U))
		{
			pxContext->uNoOpAuditCountedInCurrentCall = 1U;
			if(pxContext->xLastReport.uNoOpRefreshCount < UINT32_MAX)
			{
				pxContext->xLastReport.uNoOpRefreshCount++;
			}
			if(eTriggerChannelId == ePreferredChannelId)
			{
				if(pxContext->xLastReport.uPreferredChannelTriggeredNoOpRefreshCount < UINT32_MAX)
				{
					pxContext->xLastReport.uPreferredChannelTriggeredNoOpRefreshCount++;
				}
			}
			else if(pxContext->xLastReport.uNonPreferredChannelTriggeredNoOpRefreshCount < UINT32_MAX)
			{
				pxContext->xLastReport.uNonPreferredChannelTriggeredNoOpRefreshCount++;
			}
			if(eCurrentActiveChannelId == ePreferredChannelId)
			{
				pxContext->xLastReport.eLastSwitchReason =
					RSRX_SUPERVISOR_SWITCH_REASON_ACTIVE_REFRESH_NOOP;
				if(pxContext->xLastReport.uActiveRefreshNoOpCount < UINT32_MAX)
				{
					pxContext->xLastReport.uActiveRefreshNoOpCount++;
				}
			}
			else
			{
				pxContext->xLastReport.eLastSwitchReason =
					RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_REFRESH_NOOP;
				if(pxContext->xLastReport.uHoldoffRefreshNoOpCount < UINT32_MAX)
				{
					pxContext->xLastReport.uHoldoffRefreshNoOpCount++;
				}
				if((uPreviousHoldoffProgressCount == 0U) &&
					(pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount > 0U) &&
					(pxContext->xLastReport.uHoldoffCycleCount < UINT32_MAX))
				{
					pxContext->xLastReport.uHoldoffCycleCount++;
					pxContext->xLastReport.eLastHoldoffCycleStartTriggerEventType =
						eTriggerEventType;
					pxContext->xLastReport.eLastHoldoffCycleStartTriggerChannelId =
						eTriggerChannelId;
				}
				if(pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount > 0U)
				{
					pxContext->xLastReport.eLastHoldoffCycleState =
						RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_IN_PROGRESS;
				}
			}
		}
		else if((eTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_DOWN) &&
			(eTriggerChannelId == ePreferredChannelId) &&
			(eCurrentActiveChannelId != ePreferredChannelId) &&
			(uPreviousHoldoffProgressCount > 0U) &&
			(pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount == 0U))
		{
			pxContext->xLastReport.eLastSwitchReason =
				RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_RESET_CHANNEL_DOWN;
			if(pxContext->xLastReport.uAbortedHoldoffCycleCount < UINT32_MAX)
			{
				pxContext->xLastReport.uAbortedHoldoffCycleCount++;
			}
			if(pxContext->xLastReport.uHoldoffResetCount < UINT32_MAX)
			{
				pxContext->xLastReport.uHoldoffResetCount++;
			}
			pxContext->xLastReport.eLastHoldoffCycleState =
				RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_ABORTED;
			pxContext->xLastReport.eLastTerminalHoldoffOutcome =
				RSRX_SUPERVISOR_TERMINAL_HOLDOFF_OUTCOME_ABORTED;
			if(pxContext->xLastReport.uTerminalHoldoffOutcomeCount < UINT32_MAX)
			{
				pxContext->xLastReport.uTerminalHoldoffOutcomeCount++;
			}
			if(pxContext->xLastReport.uAbortedTerminalHoldoffOutcomeCount < UINT32_MAX)
			{
				pxContext->xLastReport.uAbortedTerminalHoldoffOutcomeCount++;
			}
			if(pxContext->xLastReport.uPreferredChannelTriggeredTerminalHoldoffOutcomeCount < UINT32_MAX)
			{
				pxContext->xLastReport.uPreferredChannelTriggeredTerminalHoldoffOutcomeCount++;
			}
			if(pxContext->xLastReport.uChannelDownTriggeredTerminalHoldoffOutcomeCount < UINT32_MAX)
			{
				pxContext->xLastReport.uChannelDownTriggeredTerminalHoldoffOutcomeCount++;
			}
			pxContext->xLastReport.eLastTerminalHoldoffOutcomeTriggerEventType =
				eTriggerEventType;
			pxContext->xLastReport.eLastTerminalHoldoffOutcomeTriggerChannelId =
				eTriggerChannelId;
		}
		else
		{
			pxContext->xLastReport.eLastSwitchReason =
				RSRX_SUPERVISOR_SWITCH_REASON_NONE;
			pxContext->xLastReport.eLastSwitchTriggerEventType =
				RSRX_TRANSPORT_EVENT_NONE;
			pxContext->xLastReport.eLastSwitchTriggerChannelId =
				RSRX_TRANSPORT_CHANNEL_INVALID;
		}
		if((pxContext->xLastReport.eLastSwitchReason == RSRX_SUPERVISOR_SWITCH_REASON_NONE) &&
			(pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount == 0U) &&
			(pxContext->xLastReport.eLastHoldoffCycleState ==
				RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_IN_PROGRESS))
		{
			pxContext->xLastReport.eLastHoldoffCycleState =
				RSRX_SUPERVISOR_HOLDOFF_CYCLE_STATE_NONE;
		}
		pxContext->xLastReport.eLastSwitchKind =
			RSRX_SUPERVISOR_SWITCH_KIND_NONE;
		pxContext->xLastReport.eLastSwitchFromChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
		pxContext->xLastReport.eLastSwitchToChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	}
}

static uint32_t uCountAvailableChannels(
	const rsrx_transport_supervisor_context_t * pxContext)
{
	const rsrx_channel_manager_context_t * pxChannelManager;
	uint32_t uIndex;
	uint32_t uCount;

	if((pxContext == (const rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (const rsrx_session_t *)0))
	{
		return 0U;
	}

	pxChannelManager = &pxContext->pxSession->xChannelManager;
	if(pxChannelManager->uInitialized == 0U)
	{
		return pxContext->xLastReport.xLastChannelState.uIsAvailable;
	}

	uCount = 0U;
	for(uIndex = 0U; uIndex < pxChannelManager->xConfig.uChannelCount; ++uIndex)
	{
		if(pxChannelManager->xConfig.axChannels[uIndex].uIsAvailable != 0U)
		{
			uCount++;
		}
	}

	return uCount;
}

static void vRefreshOutboundQueueTelemetry(
	rsrx_transport_supervisor_context_t * pxContext)
{
	const rsrx_outbound_send_telemetry_t * pxTelemetry;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (rsrx_session_t *)0))
	{
		return;
	}

	pxContext->xLastReport.uOutstandingSendPresent =
		rsrx_transport_adapter_has_outstanding_send(
			&pxContext->pxSession->xTransportAdapter);
	pxContext->xLastReport.uDeferredSendPresent =
		pxContext->pxSession->xTransportAdapter.uHasDeferredSend;
	pxContext->xLastReport.uDeferredSendCount =
		pxContext->pxSession->xTransportAdapter.uDeferredSendCount;

	pxTelemetry = rsrx_session_get_outbound_telemetry(pxContext->pxSession);
	if(pxTelemetry == (const rsrx_outbound_send_telemetry_t *)0)
	{
		pxContext->xLastReport.uDeferredSendCount = 0U;
		pxContext->xLastReport.uQueuedSendCount = 0U;
		pxContext->xLastReport.uMaxDeferredSendCount = 0U;
		pxContext->xLastReport.uDeferredDispatchCount = 0U;
		pxContext->xLastReport.uQueueOverflowRejectCount = 0U;
		pxContext->xLastReport.eLastOutboundRejectReason =
			RSRX_OUTBOUND_REJECT_REASON_NONE;
		pxContext->xLastReport.uBusyRejectedSendCount = 0U;
		pxContext->xLastReport.uConsecutiveBusyRejectedSendCount = 0U;
		pxContext->xLastReport.uMaxConsecutiveBusyRejectedSendCount = 0U;
		pxContext->xLastReport.uBusyRejectEscalationCount = 0U;
		pxContext->xLastReport.uLastBusyRejectEscalated = 0U;
		return;
	}

	pxContext->xLastReport.uQueuedSendCount = pxTelemetry->uQueuedSendCount;
	pxContext->xLastReport.uMaxDeferredSendCount =
		pxTelemetry->uMaxDeferredSendCount;
	pxContext->xLastReport.uDeferredDispatchCount = pxTelemetry->uDeferredDispatchCount;
	pxContext->xLastReport.uQueueOverflowRejectCount =
		pxTelemetry->uQueueOverflowRejectCount;
	pxContext->xLastReport.eLastOutboundRejectReason =
		pxTelemetry->eLastRejectReason;
	pxContext->xLastReport.uBusyRejectedSendCount =
		pxTelemetry->uBusyRejectedSendCount;
	pxContext->xLastReport.uConsecutiveBusyRejectedSendCount =
		pxTelemetry->uConsecutiveBusyRejectedSendCount;
	pxContext->xLastReport.uMaxConsecutiveBusyRejectedSendCount =
		pxTelemetry->uMaxConsecutiveBusyRejectedSendCount;
	pxContext->xLastReport.uBusyRejectEscalationCount =
		pxTelemetry->uBusyRejectEscalationCount;
	pxContext->xLastReport.uLastBusyRejectEscalated =
		pxTelemetry->uLastBusyRejectEscalated;
}

static rsrx_transport_channel_id_t eGetActiveChannelId(
	const rsrx_transport_supervisor_context_t * pxContext)
{
	if((pxContext == (const rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (const rsrx_session_t *)0))
	{
		return RSRX_TRANSPORT_CHANNEL_INVALID;
	}

	return rsrx_channel_manager_get_active_channel(
		&pxContext->pxSession->xChannelManager);
}

static rsrx_event_t eResolveInboundEvent(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage)
{
	rsrx_event_t eResolvedEvent;

	if(rsrx_protocol_context_resolve_inbound_event(
		&pxContext->pxSession->xTransportAdapter.xProtocolContext,
		pxMessage,
		&eResolvedEvent) != RSRX_STATUS_OK)
	{
		return RSRX_EVENT_PROTOCOL_ERROR;
	}

	return eResolvedEvent;
}

static uint32_t uSessionStatusIsHandled(
	rsrx_status_t eStatus)
{
	return (uint32_t)((eStatus == RSRX_STATUS_OK) ||
		(eStatus == RSRX_STATUS_REJECTED));
}

static rsrx_supervisor_status_t eProcessSessionEventWithDecisionInternal(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_event_t eEvent,
	rsrx_supervisor_decision_t eDecision,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_status_t eSessionStatus;

	eSessionStatus = rsrx_session_process_event(
		pxContext->pxSession,
		eEvent,
		&pxContext->xLastReport.pxLastReport);
	pxContext->xLastReport.eLastEffectiveEvent = eEvent;
	pxContext->xLastReport.eLastSessionStatus = eSessionStatus;
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}
	vRecordDecision(pxContext, eDecision);
	vRefreshChannelSwitchTelemetry(
		pxContext,
		eGetActiveChannelId(pxContext),
		RSRX_TRANSPORT_EVENT_NONE,
		RSRX_TRANSPORT_CHANNEL_INVALID);
	vRefreshOutboundQueueTelemetry(pxContext);

	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}

static void vResetSendFailureBudget(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_supervisor_budget_update_t eBudgetUpdate)
{
	if(pxContext->xLastReport.uConsecutiveSendFailureCount > 0U)
	{
		pxContext->xLastReport.uConsecutiveSendFailureCount = 0U;
		if(pxContext->xLastReport.uSendFailureBudgetResetCount < UINT32_MAX)
		{
			pxContext->xLastReport.uSendFailureBudgetResetCount++;
		}
		pxContext->xLastReport.eLastBudgetUpdate = eBudgetUpdate;
		pxContext->xLastReport.eBudgetChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	}
	else
	{
		pxContext->xLastReport.eLastBudgetUpdate = RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
	}
}

static void vResetReceiveErrorBudget(
	rsrx_transport_supervisor_context_t * pxContext)
{
	if(pxContext->xLastReport.uConsecutiveReceiveErrorCount > 0U)
	{
		pxContext->xLastReport.uConsecutiveReceiveErrorCount = 0U;
		if(pxContext->xLastReport.uReceiveErrorBudgetResetCount < UINT32_MAX)
		{
			pxContext->xLastReport.uReceiveErrorBudgetResetCount++;
		}
	}
}

static uint32_t uReceiveErrorBudgetExceeded(
	rsrx_transport_supervisor_context_t * pxContext)
{
	if(pxContext->xLastReport.uConsecutiveReceiveErrorCount < UINT32_MAX)
	{
		pxContext->xLastReport.uConsecutiveReceiveErrorCount++;
	}

	return (uint32_t)(pxContext->xLastReport.uConsecutiveReceiveErrorCount >=
		pxContext->uMaxConsecutiveReceiveErrors);
}

static rsrx_supervisor_status_t eEscalateToProtocolError(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_supervisor_decision_t eDecision,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_status_t eSessionStatus;

	eSessionStatus = rsrx_session_process_event(
		pxContext->pxSession,
		RSRX_EVENT_PROTOCOL_ERROR,
		&pxContext->xLastReport.pxLastReport);
	pxContext->xLastReport.eLastEffectiveEvent = RSRX_EVENT_PROTOCOL_ERROR;
	pxContext->xLastReport.eLastSessionStatus = eSessionStatus;
	vRecordDecision(pxContext, eDecision);
	vRefreshChannelSwitchTelemetry(
		pxContext,
		eGetActiveChannelId(pxContext),
		RSRX_TRANSPORT_EVENT_NONE,
		RSRX_TRANSPORT_CHANNEL_INVALID);
	vRefreshOutboundQueueTelemetry(pxContext);
	*ppxReport = &pxContext->xLastReport;

	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}

	return RSRX_SUPERVISOR_STATUS_OK;
}

static uint32_t uAlternativeChannelIsAvailable(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eFailedChannelId)
{
	rsrx_transport_status_t eTransportStatus;
	rsrx_transport_channel_state_t xChannelState;

	eTransportStatus = rsrx_transport_adapter_query_channel(
		&pxContext->pxSession->xTransportAdapter,
		&xChannelState);
	if((eTransportStatus != RSRX_TRANSPORT_STATUS_OK) ||
		(xChannelState.uIsAvailable == 0U))
	{
		return 0U;
	}

	pxContext->xLastReport.xLastChannelState = xChannelState;
	return (uint32_t)(xChannelState.eChannelId != eFailedChannelId);
}

static uint32_t uRefreshAvailableChannelState(
	rsrx_transport_supervisor_context_t * pxContext)
{
	rsrx_transport_status_t eTransportStatus;
	rsrx_transport_channel_state_t xChannelState;

	eTransportStatus = rsrx_transport_adapter_query_channel(
		&pxContext->pxSession->xTransportAdapter,
		&xChannelState);
	if((eTransportStatus != RSRX_TRANSPORT_STATUS_OK) ||
		(xChannelState.uIsAvailable == 0U))
	{
		return 0U;
	}

	pxContext->xLastReport.xLastChannelState = xChannelState;
	return 1U;
}

static uint32_t uFrameTargetsActiveChannel(
	const rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId)
{
	rsrx_transport_channel_id_t eActiveChannelId;

	eActiveChannelId = eGetActiveChannelId(pxContext);

	return (uint32_t)((eActiveChannelId == RSRX_TRANSPORT_CHANNEL_INVALID) ||
		(eActiveChannelId == eChannelId));
}

static uint32_t uFrameMatchesOutstandingSend(
	const rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId)
{
	rsrx_transport_channel_id_t eOutstandingChannelId;

	if((pxContext == (const rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (const rsrx_session_t *)0))
	{
		return 0U;
	}

	if(rsrx_transport_adapter_has_outstanding_send(
		&pxContext->pxSession->xTransportAdapter) == 0U)
	{
		return 0U;
	}

	eOutstandingChannelId = rsrx_transport_adapter_get_outstanding_send_channel(
		&pxContext->pxSession->xTransportAdapter);

	return (uint32_t)(eOutstandingChannelId == eChannelId);
}

static uint32_t uSendFailureBudgetExceeded(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_transport_channel_id_t eActiveChannelId)
{
	if((pxContext->xLastReport.eBudgetChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(pxContext->xLastReport.eBudgetChannelId != eActiveChannelId) &&
		(pxContext->xLastReport.uConsecutiveSendFailureCount > 0U))
	{
		pxContext->xLastReport.uConsecutiveSendFailureCount = 0U;
		if(pxContext->xLastReport.uSendFailureBudgetResetCount < UINT32_MAX)
		{
			pxContext->xLastReport.uSendFailureBudgetResetCount++;
		}
		pxContext->xLastReport.eLastBudgetUpdate =
			RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_AND_INCREMENT_ON_CHANNEL_SWITCH;
	}
	else
	{
		pxContext->xLastReport.eLastBudgetUpdate =
			RSRX_SUPERVISOR_BUDGET_UPDATE_INCREMENTED;
	}

	if(pxContext->xLastReport.uConsecutiveSendFailureCount < UINT32_MAX)
	{
		pxContext->xLastReport.uConsecutiveSendFailureCount++;
	}
	pxContext->xLastReport.eBudgetChannelId = eActiveChannelId;

	return (uint32_t)(pxContext->xLastReport.uConsecutiveSendFailureCount >=
		pxContext->uMaxConsecutiveSendFailures);
}

static rsrx_supervisor_status_t eProcessFrameInternal(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_codec_status_t eCodecStatus;
	rsrx_status_t eSessionStatus;
	rsrx_event_t eInboundEvent;

	pxContext->xLastReport.xLastFrame = *pxFrame;
	eCodecStatus = pxContext->xCodec.pfDecode(pxFrame, &pxContext->xLastReport.xLastMessage);
	if(eCodecStatus != RSRX_CODEC_STATUS_OK)
	{
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_DECODE_FAILED);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_DECODE_FAILED;
	}

	eInboundEvent = eResolveInboundEvent(
		pxContext,
		&pxContext->xLastReport.xLastMessage);
	pxContext->xLastReport.eLastEffectiveEvent = eInboundEvent;
	if((eInboundEvent == pxContext->xLastReport.xLastMessage.eSuggestedEvent) ||
		(eInboundEvent == RSRX_EVENT_RECOVERY_SUCCESS))
	{
		rsrx_transport_adapter_record_inbound_message(
			&pxContext->pxSession->xTransportAdapter,
			&pxContext->xLastReport.xLastMessage);
	}

	vResetSendFailureBudget(
		pxContext,
		RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_INBOUND_FRAME);
	pxContext->xLastReport.eLastReceiveErrorStage =
		RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_NONE;
	pxContext->xLastReport.eLastReceiveTransportStatus = RSRX_TRANSPORT_STATUS_OK;
	vResetReceiveErrorBudget(pxContext);

	eSessionStatus = rsrx_session_process_event(
		pxContext->pxSession,
		eInboundEvent,
		&pxContext->xLastReport.pxLastReport);
	pxContext->xLastReport.eLastSessionStatus = eSessionStatus;
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}
	vRecordDecision(
		pxContext,
		(eSessionStatus == RSRX_STATUS_REJECTED) ?
			RSRX_SUPERVISOR_DECISION_SESSION_REJECTED :
			RSRX_SUPERVISOR_DECISION_SESSION_ACCEPTED);
	vRefreshChannelSwitchTelemetry(
		pxContext,
		eGetActiveChannelId(pxContext),
		RSRX_TRANSPORT_EVENT_NONE,
		RSRX_TRANSPORT_CHANNEL_INVALID);
	vRefreshOutboundQueueTelemetry(pxContext);

	pxContext->xLastReport.uProcessedFrameCount++;
	*ppxReport = &pxContext->xLastReport;

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_init(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_session_t * pxSession,
	const rsrx_codec_port_t * pxCodec)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxSession == (rsrx_session_t *)0) ||
		(pxCodec == (const rsrx_codec_port_t *)0) ||
		(pxCodec->pfDecode == (rsrx_decode_frame_fn)0))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->pxSession = pxSession;
	pxContext->xCodec = *pxCodec;
	vResetSupervisorReport(&pxContext->xLastReport);
	pxContext->uMaxConsecutiveSendFailures = D_RSRX_SUPERVISOR_DEFAULT_SEND_FAILURE_BUDGET;
	pxContext->uMaxConsecutiveReceiveErrors = D_RSRX_SUPERVISOR_DEFAULT_RECEIVE_ERROR_BUDGET;
	pxContext->uNoOpAuditCountedInCurrentCall = 0U;
	pxContext->uInitialized = 1U;
	vRefreshChannelSwitchTelemetry(
		pxContext,
		eGetActiveChannelId(pxContext),
		RSRX_TRANSPORT_EVENT_NONE,
		RSRX_TRANSPORT_CHANNEL_INVALID);
	vRefreshOutboundQueueTelemetry(pxContext);

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_frame(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxFrame == (const rsrx_transport_frame_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uNoOpAuditCountedInCurrentCall = 0U;
	return eProcessFrameInternal(pxContext, pxFrame, ppxReport);
}

rsrx_supervisor_status_t rsrx_transport_supervisor_poll_receive(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_transport_status_t eTransportStatus;
	rsrx_transport_frame_t xFrame;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uNoOpAuditCountedInCurrentCall = 0U;
	eTransportStatus = rsrx_transport_adapter_query_channel(
		&pxContext->pxSession->xTransportAdapter,
		&pxContext->xLastReport.xLastChannelState);
	pxContext->xLastReport.eLastReceiveTransportStatus = eTransportStatus;
	vRefreshChannelSwitchTelemetry(
		pxContext,
		eGetActiveChannelId(pxContext),
		RSRX_TRANSPORT_EVENT_NONE,
		RSRX_TRANSPORT_CHANNEL_INVALID);
	vRefreshOutboundQueueTelemetry(pxContext);
	pxContext->xLastReport.uPollCount++;
	if(eTransportStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		if((eTransportStatus == RSRX_TRANSPORT_STATUS_CHANNEL_DOWN) ||
			(eTransportStatus == RSRX_TRANSPORT_STATUS_UNAVAILABLE))
		{
			pxContext->xLastReport.eLastReceiveErrorStage =
				RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_NONE;
			vResetReceiveErrorBudget(pxContext);
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN;
		}

		pxContext->xLastReport.eLastReceiveErrorStage =
			RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_CHANNEL_QUERY;
		if(uReceiveErrorBudgetExceeded(pxContext) == 0U)
		{
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
		}

		vResetReceiveErrorBudget(pxContext);
		return eEscalateToProtocolError(
			pxContext,
			RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED,
			ppxReport);
	}

	if(pxContext->xLastReport.xLastChannelState.uIsAvailable == 0U)
	{
		pxContext->xLastReport.eLastReceiveErrorStage =
			RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_NONE;
		vResetReceiveErrorBudget(pxContext);
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN;
	}

	xFrame.eChannelId = pxContext->xLastReport.xLastChannelState.eChannelId;
	xFrame.puPayload = (const uint8_t *)0;
	xFrame.xPayloadLength = 0U;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_NONE;

	eTransportStatus = rsrx_transport_adapter_receive_frame(
		&pxContext->pxSession->xTransportAdapter,
		&xFrame);
	pxContext->xLastReport.eLastReceiveTransportStatus = eTransportStatus;
	if(eTransportStatus == RSRX_TRANSPORT_STATUS_UNAVAILABLE)
	{
		pxContext->xLastReport.eLastReceiveErrorStage =
			RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_NONE;
		vResetReceiveErrorBudget(pxContext);
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_NO_FRAME;
	}

	if(eTransportStatus != RSRX_TRANSPORT_STATUS_OK)
	{
		if(eTransportStatus == RSRX_TRANSPORT_STATUS_CHANNEL_DOWN)
		{
			pxContext->xLastReport.eLastReceiveErrorStage =
				RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_NONE;
			vResetReceiveErrorBudget(pxContext);
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN;
		}

		pxContext->xLastReport.eLastReceiveErrorStage =
			RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_FRAME_RECEIVE;
		if(uReceiveErrorBudgetExceeded(pxContext) == 0U)
		{
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_BUDGETED);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
		}

		vResetReceiveErrorBudget(pxContext);
		return eEscalateToProtocolError(
			pxContext,
			RSRX_SUPERVISOR_DECISION_RECEIVE_ERROR_ESCALATED,
			ppxReport);
	}

	if(xFrame.eEventType != RSRX_TRANSPORT_EVENT_FRAME_RECEIVED)
	{
		pxContext->xLastReport.eLastReceiveErrorStage =
			RSRX_SUPERVISOR_RECEIVE_ERROR_STAGE_NONE;
		vResetReceiveErrorBudget(pxContext);
		pxContext->xLastReport.xLastFrame = xFrame;
		vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE);
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_NO_FRAME;
	}

	return eProcessFrameInternal(pxContext, &xFrame, ppxReport);
}

rsrx_supervisor_status_t rsrx_transport_supervisor_pump_receive(
	rsrx_transport_supervisor_context_t * pxContext,
	uint32_t uMaxPolls,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_supervisor_status_t eStatus;
	uint32_t uInitialProcessedCount;
	uint32_t uIteration;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U) ||
		(uMaxPolls == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xLastReport.uLastPumpIterationCount = 0U;
	pxContext->xLastReport.uLastPumpProcessedFrameCount = 0U;
	pxContext->uNoOpAuditCountedInCurrentCall = 0U;
	uInitialProcessedCount = pxContext->xLastReport.uProcessedFrameCount;

	for(uIteration = 0U; uIteration < uMaxPolls; ++uIteration)
	{
		eStatus = rsrx_transport_supervisor_poll_receive(pxContext, ppxReport);
		pxContext->xLastReport.uLastPumpIterationCount++;

		if(eStatus == RSRX_SUPERVISOR_STATUS_OK)
		{
			continue;
		}

		if(eStatus == RSRX_SUPERVISOR_STATUS_NO_FRAME)
		{
			pxContext->xLastReport.uLastPumpProcessedFrameCount =
				pxContext->xLastReport.uProcessedFrameCount - uInitialProcessedCount;
			*ppxReport = &pxContext->xLastReport;
			return ((pxContext->xLastReport.uLastPumpProcessedFrameCount > 0U) ?
				RSRX_SUPERVISOR_STATUS_OK :
				RSRX_SUPERVISOR_STATUS_NO_FRAME);
		}

		pxContext->xLastReport.uLastPumpProcessedFrameCount =
			pxContext->xLastReport.uProcessedFrameCount - uInitialProcessedCount;
		*ppxReport = &pxContext->xLastReport;
		return eStatus;
	}

	pxContext->xLastReport.uLastPumpProcessedFrameCount =
		pxContext->xLastReport.uProcessedFrameCount - uInitialProcessedCount;
	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_transport_event(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_transport_channel_id_t ePreviousActiveChannelId;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxFrame == (const rsrx_transport_frame_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uNoOpAuditCountedInCurrentCall = 0U;
	pxContext->xLastReport.xLastFrame = *pxFrame;
	ePreviousActiveChannelId = eGetActiveChannelId(pxContext);

	switch(pxFrame->eEventType)
	{
		case RSRX_TRANSPORT_EVENT_SEND_COMPLETED:
			if(uFrameMatchesOutstandingSend(pxContext, pxFrame->eChannelId) == 0U)
			{
			pxContext->xLastReport.eLastBudgetUpdate =
					RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED);
				vRefreshOutboundQueueTelemetry(pxContext);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			rsrx_transport_adapter_clear_outstanding_send_on_feedback(
				&pxContext->pxSession->xTransportAdapter);
			vResetSendFailureBudget(
				pxContext,
				RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_SEND_COMPLETED);
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_SEND_COMPLETED_IGNORED);
			vRefreshOutboundQueueTelemetry(pxContext);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;

		case RSRX_TRANSPORT_EVENT_SEND_FAILED:
			if(uFrameMatchesOutstandingSend(pxContext, pxFrame->eChannelId) == 0U)
			{
				pxContext->xLastReport.eLastBudgetUpdate =
					RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_SEND_FEEDBACK_UNCORRELATED_IGNORED);
				vRefreshOutboundQueueTelemetry(pxContext);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			if(uFrameTargetsActiveChannel(pxContext, pxFrame->eChannelId) == 0U)
			{
				pxContext->xLastReport.eLastBudgetUpdate =
					RSRX_SUPERVISOR_BUDGET_UPDATE_NONE;
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_SEND_FAILURE_INACTIVE_CHANNEL_IGNORED);
				vRefreshOutboundQueueTelemetry(pxContext);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			if(uSendFailureBudgetExceeded(
				pxContext,
				eGetActiveChannelId(pxContext)) == 0U)
			{
				vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_SEND_FAILURE_BUDGETED);
				vRefreshOutboundQueueTelemetry(pxContext);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}

			vResetSendFailureBudget(
				pxContext,
				RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_ESCALATION);
			rsrx_transport_adapter_clear_outstanding_send_on_feedback(
				&pxContext->pxSession->xTransportAdapter);
			return eProcessSessionEventWithDecisionInternal(
				pxContext,
				RSRX_EVENT_PROTOCOL_ERROR,
				RSRX_SUPERVISOR_DECISION_SEND_FAILURE_ESCALATED,
				ppxReport);

		case RSRX_TRANSPORT_EVENT_CHANNEL_DOWN:
			vResetSendFailureBudget(
				pxContext,
				RSRX_SUPERVISOR_BUDGET_UPDATE_RESET_ON_CHANNEL_DOWN);
			if(uAlternativeChannelIsAvailable(pxContext, pxFrame->eChannelId) != 0U)
			{
				vRecordDecision(
					pxContext,
					RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_FAILOVER_USED);
				vRefreshChannelSwitchTelemetry(
					pxContext,
					ePreviousActiveChannelId,
					pxFrame->eEventType,
					pxFrame->eChannelId);
				vRefreshOutboundQueueTelemetry(pxContext);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}
			return eProcessSessionEventWithDecisionInternal(
				pxContext,
				RSRX_EVENT_PROTOCOL_ERROR,
				RSRX_SUPERVISOR_DECISION_CHANNEL_DOWN_ESCALATED,
				ppxReport);

		case RSRX_TRANSPORT_EVENT_CHANNEL_UP:
			if(uRefreshAvailableChannelState(pxContext) != 0U)
			{
				vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_CHANNEL_UP_REFRESHED);
				vRefreshChannelSwitchTelemetry(
					pxContext,
					ePreviousActiveChannelId,
					pxFrame->eEventType,
					pxFrame->eChannelId);
				vRefreshOutboundQueueTelemetry(pxContext);
				*ppxReport = &pxContext->xLastReport;
				return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
			}
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_TRANSPORT_EVENT_IGNORED);
			vRefreshChannelSwitchTelemetry(
				pxContext,
				ePreviousActiveChannelId,
				pxFrame->eEventType,
				pxFrame->eChannelId);
			vRefreshOutboundQueueTelemetry(pxContext);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;

		case RSRX_TRANSPORT_EVENT_FRAME_RECEIVED:
			return eProcessFrameInternal(pxContext, pxFrame, ppxReport);

		case RSRX_TRANSPORT_EVENT_NONE:
		default:
			vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_TRANSPORT_EVENT_IGNORED);
			vRefreshChannelSwitchTelemetry(
				pxContext,
				ePreviousActiveChannelId,
				pxFrame->eEventType,
				pxFrame->eChannelId);
			vRefreshOutboundQueueTelemetry(pxContext);
			*ppxReport = &pxContext->xLastReport;
			return RSRX_SUPERVISOR_STATUS_IGNORED_EVENT;
	}
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_timer_expiry(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_timer_expiry_source_t eTimerSource,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	rsrx_status_t eSessionStatus;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(ppxReport == (const rsrx_transport_supervisor_report_t **)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uNoOpAuditCountedInCurrentCall = 0U;
	eSessionStatus = rsrx_session_process_timer_expiry(
		pxContext->pxSession,
		eTimerSource,
		&pxContext->xLastReport.pxLastReport);
	pxContext->xLastReport.eLastSessionStatus = eSessionStatus;
	vRecordDecision(pxContext, RSRX_SUPERVISOR_DECISION_TIMER_DELEGATED);
	vRefreshChannelSwitchTelemetry(
		pxContext,
		eGetActiveChannelId(pxContext),
		RSRX_TRANSPORT_EVENT_NONE,
		RSRX_TRANSPORT_CHANNEL_INVALID);
	vRefreshOutboundQueueTelemetry(pxContext);
	if(uSessionStatusIsHandled(eSessionStatus) == 0U)
	{
		*ppxReport = &pxContext->xLastReport;
		return RSRX_SUPERVISOR_STATUS_SESSION_ERROR;
	}

	*ppxReport = &pxContext->xLastReport;
	return RSRX_SUPERVISOR_STATUS_OK;
}
