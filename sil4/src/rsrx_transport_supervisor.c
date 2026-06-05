#include "rsrx_transport_supervisor.h"

#define D_RSRX_SUPERVISOR_DEFAULT_SEND_FAILURE_BUDGET (2U)
#define D_RSRX_SUPERVISOR_DEFAULT_RECEIVE_ERROR_BUDGET (2U)

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
	pxReport->eLastCodecStatus = RSRX_CODEC_STATUS_OK;
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
	pxReport->uPreferredChannelTriggeredHoldoffCycleCount = 0U;
	pxReport->uNonPreferredChannelTriggeredHoldoffCycleCount = 0U;
	pxReport->uChannelUpTriggeredHoldoffCycleCount = 0U;
	pxReport->uChannelDownTriggeredHoldoffCycleCount = 0U;
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
	pxReport->uPreferredRecoveryPenaltyRearmCount = 0U;
	pxReport->uPreferredRecoveryPenaltyAppliedCycleCount = 0U;
	pxReport->uPreferredRecoveryPenaltyAbortCount = 0U;
	pxReport->uPreferredRecoveryPenaltyClearCount = 0U;
	pxReport->uPreferredRecoveryPenaltyBypassClearCount = 0U;
	pxReport->uPreferredRecoveryPenaltyResetClearCount = 0U;
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
	pxReport->uOutboundRuntimeResetCount = 0U;
	pxReport->eLastOutboundRejectReason = RSRX_OUTBOUND_REJECT_REASON_NONE;
	pxReport->uBusyRejectedSendCount = 0U;
	pxReport->uConsecutiveBusyRejectedSendCount = 0U;
	pxReport->uMaxConsecutiveBusyRejectedSendCount = 0U;
	pxReport->uBusyRejectEscalationCount = 0U;
	pxReport->uLastBusyRejectEscalated = 0U;
	pxReport->uRastaSrRuntimeEnabled = 0U;
	pxReport->uRastaRedundancySrRuntimeEnabled = 0U;
	pxReport->uRastaSrCurrentTimestamp = 0U;
	pxReport->uRastaSrLastAcceptedTimestamp = 0U;
	pxReport->uRastaSrIdentityAdmissionEnabled = 0U;
	pxReport->uRastaSrExpectedReceiverId = 0U;
	pxReport->uRastaSrExpectedSenderId = 0U;
}

static void vRefreshRastaSrRuntimeTelemetry(
	rsrx_transport_supervisor_context_t * pxContext)
{
	pxContext->xLastReport.uRastaSrRuntimeEnabled =
		pxContext->uRastaSrRuntimeEnabled;
	pxContext->xLastReport.uRastaRedundancySrRuntimeEnabled =
		pxContext->uRastaRedundancySrRuntimeEnabled;
	pxContext->xLastReport.uRastaSrCurrentTimestamp =
		pxContext->xRastaSrTimestampPolicy.uCurrentTimestamp;
	pxContext->xLastReport.uRastaSrLastAcceptedTimestamp =
		pxContext->xRastaSrTimestampPolicy.uLastAcceptedTimestamp;
	pxContext->xLastReport.uRastaSrIdentityAdmissionEnabled =
		pxContext->uRastaSrIdentityAdmissionEnabled;
	pxContext->xLastReport.uRastaSrExpectedReceiverId =
		pxContext->xRastaSrIdentityPolicy.uExpectedReceiverId;
	pxContext->xLastReport.uRastaSrExpectedSenderId =
		pxContext->xRastaSrIdentityPolicy.uExpectedSenderId;
}

static uint32_t uRastaSrTimestampPolicyHasStableBounds(
	const rsrx_rasta_sr_timestamp_admission_policy_t * pxPolicy)
{
	if(pxPolicy->uCurrentTimestamp == 0U)
	{
		return 0U;
	}
	if(pxPolicy->uCurrentTimestamp < pxPolicy->uAcceptedPastWindow)
	{
		return 0U;
	}
	if((UINT32_MAX - pxPolicy->uCurrentTimestamp) <
		pxPolicy->uAcceptedFutureWindow)
	{
		return 0U;
	}

	return 1U;
}

static uint32_t uRastaSrIdentityPolicyHasStableIds(
	const rsrx_rasta_sr_identity_admission_policy_t * pxPolicy)
{
	return (uint32_t)((pxPolicy->uExpectedReceiverId != 0U) &&
		(pxPolicy->uExpectedSenderId != 0U));
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
	rsrx_channel_manager_snapshot_t xSnapshot;
	uint32_t uPreviousHoldoffProgressCount;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (rsrx_session_t *)0))
	{
		return;
	}

	if(rsrx_session_copy_channel_manager_snapshot(
		pxContext->pxSession,
		&xSnapshot) != RSRX_STATUS_OK)
	{
		return;
	}

	eCurrentActiveChannelId = xSnapshot.eActiveChannelId;
	uPreviousHoldoffProgressCount =
		pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount;
	ePreferredChannelId = xSnapshot.ePreferredChannelId;
	pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount =
		xSnapshot.uPreferredRecoveryStableSelectionCount;
	pxContext->xLastReport.uPreferredRecoveryPendingPenaltyCount =
		xSnapshot.uPreferredRecoveryPendingPenaltySelections;
	pxContext->xLastReport.uPreferredRecoveryPenaltyArmCount =
		xSnapshot.uPreferredRecoveryPenaltyArmCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyRearmCount =
		xSnapshot.uPreferredRecoveryPenaltyRearmCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyAppliedCycleCount =
		xSnapshot.uPreferredRecoveryPenaltyAppliedCycleCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyAbortCount =
		xSnapshot.uPreferredRecoveryPenaltyAbortCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyClearCount =
		xSnapshot.uPreferredRecoveryPenaltyClearCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyBypassClearCount =
		xSnapshot.uPreferredRecoveryPenaltyBypassClearCount;
	pxContext->xLastReport.uPreferredRecoveryPenaltyResetClearCount =
		xSnapshot.uPreferredRecoveryPenaltyResetClearCount;
	pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount =
		xSnapshot.uPreferredRecoveryHoldoffTargetCount;
	pxContext->xLastReport.uPreferredRecoveryHoldoffRemainingCount =
		xSnapshot.uPreferredRecoveryHoldoffRemainingCount;
	pxContext->xLastReport.uAvailableChannelCount = xSnapshot.uAvailableChannelCount;
	pxContext->xLastReport.uPreferredRecoveryHoldoffActive = (uint32_t)(
		(pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount > 0U) &&
		(pxContext->xLastReport.uPreferredRecoveryHoldoffProgressCount <
			pxContext->xLastReport.uPreferredRecoveryHoldoffTargetCount) &&
		(eCurrentActiveChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(ePreferredChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(eCurrentActiveChannelId != ePreferredChannelId) &&
		(pxContext->xLastReport.uAvailableChannelCount > 1U));
	pxContext->xLastReport.uChannelSwitchCount =
		xSnapshot.uTotalSwitchCount;
	pxContext->xLastReport.uLastChannelSwitchOccurred =
		xSnapshot.uLastSelectionWasFailover;
	pxContext->xLastReport.uChannelUnavailableSelectionCount =
		xSnapshot.uUnavailableSelectionCount;
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
					if(eTriggerChannelId == ePreferredChannelId)
					{
						if(pxContext->xLastReport.uPreferredChannelTriggeredHoldoffCycleCount < UINT32_MAX)
						{
							pxContext->xLastReport.uPreferredChannelTriggeredHoldoffCycleCount++;
						}
					}
					else if(pxContext->xLastReport.uNonPreferredChannelTriggeredHoldoffCycleCount < UINT32_MAX)
					{
						pxContext->xLastReport.uNonPreferredChannelTriggeredHoldoffCycleCount++;
					}
					if(eTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_UP)
					{
						if(pxContext->xLastReport.uChannelUpTriggeredHoldoffCycleCount < UINT32_MAX)
						{
							pxContext->xLastReport.uChannelUpTriggeredHoldoffCycleCount++;
						}
					}
					else if((eTriggerEventType == RSRX_TRANSPORT_EVENT_CHANNEL_DOWN) &&
						(pxContext->xLastReport.uChannelDownTriggeredHoldoffCycleCount < UINT32_MAX))
					{
						pxContext->xLastReport.uChannelDownTriggeredHoldoffCycleCount++;
					}
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

static void vRefreshOutboundQueueTelemetry(
	rsrx_transport_supervisor_context_t * pxContext)
{
	rsrx_outbound_queue_snapshot_t xSnapshot;

	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (rsrx_session_t *)0))
	{
		return;
	}

	if(rsrx_session_copy_outbound_queue_snapshot(
		pxContext->pxSession,
		&xSnapshot) != RSRX_STATUS_OK)
	{
		pxContext->xLastReport.uOutstandingSendPresent = 0U;
		pxContext->xLastReport.uDeferredSendPresent = 0U;
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

	pxContext->xLastReport.uOutstandingSendPresent =
		xSnapshot.uOutstandingSendPresent;
	pxContext->xLastReport.uDeferredSendPresent =
		xSnapshot.uDeferredSendPresent;
	pxContext->xLastReport.uDeferredSendCount =
		xSnapshot.uDeferredSendCount;
	pxContext->xLastReport.uQueuedSendCount =
		xSnapshot.xTelemetry.uQueuedSendCount;
	pxContext->xLastReport.uMaxDeferredSendCount =
		xSnapshot.xTelemetry.uMaxDeferredSendCount;
	pxContext->xLastReport.uDeferredDispatchCount =
		xSnapshot.xTelemetry.uDeferredDispatchCount;
	pxContext->xLastReport.uQueueOverflowRejectCount =
		xSnapshot.xTelemetry.uQueueOverflowRejectCount;
	pxContext->xLastReport.uOutboundRuntimeResetCount =
		xSnapshot.xTelemetry.uRuntimeResetCount;
	pxContext->xLastReport.eLastOutboundRejectReason =
		xSnapshot.xTelemetry.eLastRejectReason;
	pxContext->xLastReport.uBusyRejectedSendCount =
		xSnapshot.xTelemetry.uBusyRejectedSendCount;
	pxContext->xLastReport.uConsecutiveBusyRejectedSendCount =
		xSnapshot.xTelemetry.uConsecutiveBusyRejectedSendCount;
	pxContext->xLastReport.uMaxConsecutiveBusyRejectedSendCount =
		xSnapshot.xTelemetry.uMaxConsecutiveBusyRejectedSendCount;
	pxContext->xLastReport.uBusyRejectEscalationCount =
		xSnapshot.xTelemetry.uBusyRejectEscalationCount;
	pxContext->xLastReport.uLastBusyRejectEscalated =
		xSnapshot.xTelemetry.uLastBusyRejectEscalated;
}

static rsrx_transport_channel_id_t eGetActiveChannelId(
	const rsrx_transport_supervisor_context_t * pxContext)
{
	rsrx_channel_manager_snapshot_t xSnapshot;

	if((pxContext == (const rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (const rsrx_session_t *)0))
	{
		return RSRX_TRANSPORT_CHANNEL_INVALID;
	}

	if(rsrx_session_copy_channel_manager_snapshot(
		pxContext->pxSession,
		&xSnapshot) != RSRX_STATUS_OK)
	{
		return RSRX_TRANSPORT_CHANNEL_INVALID;
	}

	return xSnapshot.eActiveChannelId;
}

static rsrx_event_t eResolveInboundEvent(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage)
{
	rsrx_event_t eResolvedEvent;

	if(rsrx_session_resolve_inbound_event(
		pxContext->pxSession,
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
	rsrx_outbound_queue_snapshot_t xSnapshot;

	if((pxContext == (const rsrx_transport_supervisor_context_t *)0) ||
		(pxContext->pxSession == (const rsrx_session_t *)0))
	{
		return 0U;
	}

	if(rsrx_session_copy_outbound_queue_snapshot(
		pxContext->pxSession,
		&xSnapshot) != RSRX_STATUS_OK)
	{
		return 0U;
	}

	if(xSnapshot.uOutstandingSendPresent == 0U)
	{
		return 0U;
	}

	return (uint32_t)(xSnapshot.eOutstandingSendChannelId == eChannelId);
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

static rsrx_codec_status_t eDecodeInboundFrame(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage)
{
	rsrx_codec_status_t eStatus;
	rsrx_rasta_sr_decoded_packet_t xPacket;

	if(pxContext->uRastaSrRuntimeEnabled == 0U)
	{
		return pxContext->xCodec.pfDecode(pxFrame, pxMessage);
	}

	if(pxContext->uRastaRedundancySrRuntimeEnabled != 0U)
	{
		eStatus = rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum(pxFrame, &xPacket);
	}
	else
	{
		eStatus = rsrx_codec_decode_rasta_sr_no_checksum(pxFrame, &xPacket);
	}
	if(eStatus != RSRX_CODEC_STATUS_OK)
	{
		return eStatus;
	}

	if(pxContext->uRastaSrIdentityAdmissionEnabled != 0U)
	{
		eStatus = rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
			&xPacket,
			&pxContext->xRastaSrTimestampPolicy,
			&pxContext->xRastaSrIdentityPolicy,
			pxMessage);
	}
	else
	{
		eStatus = rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission(
			&xPacket,
			&pxContext->xRastaSrTimestampPolicy,
			pxMessage);
	}
	if(eStatus == RSRX_CODEC_STATUS_OK)
	{
		pxContext->xRastaSrTimestampPolicy.uLastAcceptedTimestamp =
			xPacket.uTimestamp;
	}

	return eStatus;
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
	eCodecStatus = eDecodeInboundFrame(
		pxContext,
		pxFrame,
		&pxContext->xLastReport.xLastMessage);
	pxContext->xLastReport.eLastCodecStatus = eCodecStatus;
	vRefreshRastaSrRuntimeTelemetry(pxContext);
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
	pxContext->xRastaSrTimestampPolicy.uCurrentTimestamp = 0U;
	pxContext->xRastaSrTimestampPolicy.uAcceptedPastWindow = 0U;
	pxContext->xRastaSrTimestampPolicy.uAcceptedFutureWindow = 0U;
	pxContext->xRastaSrTimestampPolicy.uLastAcceptedTimestamp = 0U;
	pxContext->xRastaSrIdentityPolicy.uExpectedReceiverId = 0U;
	pxContext->xRastaSrIdentityPolicy.uExpectedSenderId = 0U;
	pxContext->uRastaSrRuntimeEnabled = 0U;
	pxContext->uRastaRedundancySrRuntimeEnabled = 0U;
	pxContext->uRastaSrIdentityAdmissionEnabled = 0U;
	pxContext->uMaxConsecutiveSendFailures = D_RSRX_SUPERVISOR_DEFAULT_SEND_FAILURE_BUDGET;
	pxContext->uMaxConsecutiveReceiveErrors = D_RSRX_SUPERVISOR_DEFAULT_RECEIVE_ERROR_BUDGET;
	pxContext->uNoOpAuditCountedInCurrentCall = 0U;
	pxContext->uInitialized = 1U;
	vRefreshRastaSrRuntimeTelemetry(pxContext);
	vRefreshChannelSwitchTelemetry(
		pxContext,
		eGetActiveChannelId(pxContext),
		RSRX_TRANSPORT_EVENT_NONE,
		RSRX_TRANSPORT_CHANNEL_INVALID);
	vRefreshOutboundQueueTelemetry(pxContext);

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_enable_rasta_sr_runtime(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_rasta_sr_timestamp_admission_policy_t * pxPolicy)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxPolicy == (const rsrx_rasta_sr_timestamp_admission_policy_t *)0) ||
		(pxContext->uInitialized == 0U) ||
		(uRastaSrTimestampPolicyHasStableBounds(pxPolicy) == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xRastaSrTimestampPolicy = *pxPolicy;
	pxContext->uRastaSrRuntimeEnabled = 1U;
	pxContext->uRastaRedundancySrRuntimeEnabled = 0U;
	vRefreshRastaSrRuntimeTelemetry(pxContext);

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_enable_rasta_redundancy_sr_runtime(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_rasta_sr_timestamp_admission_policy_t * pxPolicy)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxPolicy == (const rsrx_rasta_sr_timestamp_admission_policy_t *)0) ||
		(pxContext->uInitialized == 0U) ||
		(uRastaSrTimestampPolicyHasStableBounds(pxPolicy) == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xRastaSrTimestampPolicy = *pxPolicy;
	pxContext->uRastaSrRuntimeEnabled = 1U;
	pxContext->uRastaRedundancySrRuntimeEnabled = 1U;
	vRefreshRastaSrRuntimeTelemetry(pxContext);

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_enable_rasta_sr_identity_admission(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_rasta_sr_identity_admission_policy_t * pxPolicy)
{
	if((pxContext == (rsrx_transport_supervisor_context_t *)0) ||
		(pxPolicy == (const rsrx_rasta_sr_identity_admission_policy_t *)0) ||
		(pxContext->uInitialized == 0U) ||
		(pxContext->uRastaSrRuntimeEnabled == 0U) ||
		(uRastaSrIdentityPolicyHasStableIds(pxPolicy) == 0U))
	{
		return RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xRastaSrIdentityPolicy = *pxPolicy;
	pxContext->uRastaSrIdentityAdmissionEnabled = 1U;
	vRefreshRastaSrRuntimeTelemetry(pxContext);

	return RSRX_SUPERVISOR_STATUS_OK;
}

rsrx_supervisor_status_t rsrx_transport_supervisor_process_frame(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport)
{
	if(ppxReport != (const rsrx_transport_supervisor_report_t **)0)
	{
		*ppxReport = (const rsrx_transport_supervisor_report_t *)0;
	}

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

	if(ppxReport != (const rsrx_transport_supervisor_report_t **)0)
	{
		*ppxReport = (const rsrx_transport_supervisor_report_t *)0;
	}

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

	if(ppxReport != (const rsrx_transport_supervisor_report_t **)0)
	{
		*ppxReport = (const rsrx_transport_supervisor_report_t *)0;
	}

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

	if(ppxReport != (const rsrx_transport_supervisor_report_t **)0)
	{
		*ppxReport = (const rsrx_transport_supervisor_report_t *)0;
	}

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

	if(ppxReport != (const rsrx_transport_supervisor_report_t **)0)
	{
		*ppxReport = (const rsrx_transport_supervisor_report_t *)0;
	}

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
