#include "rsrx_channel_manager.h"

static uint32_t uGetEffectiveHoldoffTarget(
	const rsrx_channel_manager_context_t * pxContext)
{
	uint32_t uTarget;

	uTarget = pxContext->xConfig.uPreferredRecoveryHoldoffSelections;
	if((UINT32_MAX - uTarget) < pxContext->uPreferredRecoveryPendingPenaltySelections)
	{
		return UINT32_MAX;
	}

	return uTarget + pxContext->uPreferredRecoveryPendingPenaltySelections;
}

static uint32_t uConfigIsValid(
	const rsrx_channel_manager_config_t * pxConfig)
{
	uint32_t uIndex;
	uint32_t uCompareIndex;

	if((pxConfig == (const rsrx_channel_manager_config_t *)0) ||
		(pxConfig->uChannelCount == 0U) ||
		(pxConfig->uChannelCount > D_RSRX_CHANNEL_MANAGER_MAX_CHANNELS) ||
		(pxConfig->uPreferredChannelIndex >= pxConfig->uChannelCount))
	{
		return 0U;
	}

	if((pxConfig->eMode != RSRX_REDUNDANCY_MODE_SINGLE) &&
		(pxConfig->eMode != RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY))
	{
		return 0U;
	}

	if((pxConfig->eMode == RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY) &&
		(pxConfig->uChannelCount < D_RSRX_CHANNEL_MANAGER_MAX_CHANNELS))
	{
		return 0U;
	}

	for(uIndex = 0U; uIndex < pxConfig->uChannelCount; ++uIndex)
	{
		if(pxConfig->axChannels[uIndex].eChannelId == RSRX_TRANSPORT_CHANNEL_INVALID)
		{
			return 0U;
		}

		for(uCompareIndex = uIndex + 1U; uCompareIndex < pxConfig->uChannelCount; ++uCompareIndex)
		{
			if(pxConfig->axChannels[uIndex].eChannelId ==
				pxConfig->axChannels[uCompareIndex].eChannelId)
			{
				return 0U;
			}

			if(pxConfig->axChannels[uIndex].uPriority ==
				pxConfig->axChannels[uCompareIndex].uPriority)
			{
				return 0U;
			}
		}
	}

	return 1U;
}

static uint32_t uCountAvailableChannels(
	const rsrx_channel_manager_context_t * pxContext)
{
	uint32_t uIndex;
	uint32_t uCount;

	uCount = 0U;
	for(uIndex = 0U; uIndex < pxContext->xConfig.uChannelCount; ++uIndex)
	{
		if(pxContext->xConfig.axChannels[uIndex].uIsAvailable != 0U)
		{
			uCount++;
		}
	}

	return uCount;
}

static uint32_t uFindBestAvailableChannel(
	const rsrx_channel_manager_context_t * pxContext,
	uint32_t * puChannelIndex)
{
	uint32_t uIndex;
	uint32_t uBestIndex;
	uint32_t uBestPriority;
	uint32_t uFound;

	uFound = 0U;
	uBestIndex = 0U;
	uBestPriority = 0U;

	for(uIndex = 0U; uIndex < pxContext->xConfig.uChannelCount; ++uIndex)
	{
		if(pxContext->xConfig.axChannels[uIndex].uIsAvailable == 0U)
		{
			continue;
		}

		if((uFound == 0U) ||
			(pxContext->xConfig.axChannels[uIndex].uPriority < uBestPriority))
		{
			uBestIndex = uIndex;
			uBestPriority = pxContext->xConfig.axChannels[uIndex].uPriority;
			uFound = 1U;
		}
	}

	if(uFound != 0U)
	{
		*puChannelIndex = uBestIndex;
	}

	return uFound;
}

static void vPopulateHoldoffTelemetry(
	const rsrx_channel_manager_context_t * pxContext,
	rsrx_channel_selection_result_t * pxResult)
{
	uint32_t uPreferredIndex;
	uint32_t uActiveIsAvailable;
	uint32_t uPreferredIsAvailable;

	uPreferredIndex = pxContext->xConfig.uPreferredChannelIndex;
	uActiveIsAvailable = (uint32_t)(
		(pxContext->uActiveChannelIndex < pxContext->xConfig.uChannelCount) &&
		(pxContext->xConfig.axChannels[pxContext->uActiveChannelIndex].uIsAvailable != 0U));
	uPreferredIsAvailable = (uint32_t)(
		(uPreferredIndex < pxContext->xConfig.uChannelCount) &&
		(pxContext->xConfig.axChannels[uPreferredIndex].uIsAvailable != 0U));

	pxResult->uPreferredRecoveryHoldoffProgressCount =
		pxContext->uPreferredRecoveryStableSelectionCount;
	pxResult->uPreferredRecoveryHoldoffTargetCount =
		uGetEffectiveHoldoffTarget(pxContext);
	pxResult->uPreferredRecoveryHoldoffRemainingCount =
		(pxResult->uPreferredRecoveryHoldoffTargetCount >
			pxResult->uPreferredRecoveryHoldoffProgressCount) ?
			(pxResult->uPreferredRecoveryHoldoffTargetCount -
				pxResult->uPreferredRecoveryHoldoffProgressCount) :
			0U;
	pxResult->uPreferredRecoveryHoldoffActive = (uint32_t)(
		(pxContext->xConfig.eMode == RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY) &&
		(pxResult->uPreferredRecoveryHoldoffTargetCount > 0U) &&
		(uActiveIsAvailable != 0U) &&
		(uPreferredIsAvailable != 0U) &&
		(uPreferredIndex != pxContext->uActiveChannelIndex));
}

rsrx_channel_manager_status_t rsrx_channel_manager_init(
	rsrx_channel_manager_context_t * pxContext,
	const rsrx_channel_manager_config_t * pxConfig)
{
	if((pxContext == (rsrx_channel_manager_context_t *)0) ||
		(uConfigIsValid(pxConfig) == 0U))
	{
		return RSRX_CHANNEL_MANAGER_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xConfig = *pxConfig;
	pxContext->uActiveChannelIndex = pxConfig->uPreferredChannelIndex;
	pxContext->uLastSelectionWasFailover = 0U;
	pxContext->uPreferredRecoveryStableSelectionCount = 0U;
	pxContext->uPreferredRecoveryPendingPenaltySelections = 0U;
	pxContext->uTotalSwitchCount = 0U;
	pxContext->uUnavailableSelectionCount = 0U;
	pxContext->uInitialized = 1U;

	return RSRX_CHANNEL_MANAGER_STATUS_OK;
}

rsrx_channel_manager_status_t rsrx_channel_manager_update_channel(
	rsrx_channel_manager_context_t * pxContext,
	uint32_t uChannelIndex,
	const rsrx_transport_channel_state_t * pxState)
{
	if((pxContext == (rsrx_channel_manager_context_t *)0) ||
		(pxState == (const rsrx_transport_channel_state_t *)0) ||
		(pxContext->uInitialized == 0U) ||
		(uChannelIndex >= pxContext->xConfig.uChannelCount) ||
		(pxState->eChannelId != pxContext->xConfig.axChannels[uChannelIndex].eChannelId))
	{
		return RSRX_CHANNEL_MANAGER_STATUS_INVALID_ARGUMENT;
	}

	pxContext->xConfig.axChannels[uChannelIndex].uIsAvailable = pxState->uIsAvailable;

	return RSRX_CHANNEL_MANAGER_STATUS_OK;
}

rsrx_channel_manager_status_t rsrx_channel_manager_select_channel(
	rsrx_channel_manager_context_t * pxContext,
	rsrx_channel_selection_result_t * pxResult)
{
	uint32_t uSelectedIndex;
	uint32_t uPreviousIndex;
	uint32_t uPreferredIndex;
	uint32_t uActiveIsAvailable;
	uint32_t uPreferredIsAvailable;
	uint32_t uEffectiveHoldoffTarget;
	uint32_t uHadHoldoffProgress;

	if((pxContext == (rsrx_channel_manager_context_t *)0) ||
		(pxResult == (rsrx_channel_selection_result_t *)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_CHANNEL_MANAGER_STATUS_INVALID_ARGUMENT;
	}

	uPreviousIndex = pxContext->uActiveChannelIndex;
	pxContext->uLastSelectionWasFailover = 0U;
	uPreferredIndex = pxContext->xConfig.uPreferredChannelIndex;
	uHadHoldoffProgress =
		(uint32_t)(pxContext->uPreferredRecoveryStableSelectionCount > 0U);
	uActiveIsAvailable = (uint32_t)(
		(pxContext->uActiveChannelIndex < pxContext->xConfig.uChannelCount) &&
		(pxContext->xConfig.axChannels[pxContext->uActiveChannelIndex].uIsAvailable != 0U));
	uPreferredIsAvailable = (uint32_t)(
		(uPreferredIndex < pxContext->xConfig.uChannelCount) &&
		(pxContext->xConfig.axChannels[uPreferredIndex].uIsAvailable != 0U));
	uEffectiveHoldoffTarget = uGetEffectiveHoldoffTarget(pxContext);

	if((pxContext->xConfig.eMode == RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY) &&
		(uPreferredIsAvailable != 0U) &&
		(uPreferredIndex != pxContext->uActiveChannelIndex))
	{
		if((uActiveIsAvailable == 0U) ||
			(uEffectiveHoldoffTarget == 0U))
		{
			uSelectedIndex = uPreferredIndex;
			pxContext->uPreferredRecoveryStableSelectionCount = 0U;
			pxContext->uPreferredRecoveryPendingPenaltySelections = 0U;
			pxContext->uLastSelectionWasFailover =
				(uint32_t)(uSelectedIndex != uPreviousIndex);
		}
		else
		{
			if(pxContext->uPreferredRecoveryStableSelectionCount < UINT32_MAX)
			{
				pxContext->uPreferredRecoveryStableSelectionCount++;
			}

			if(pxContext->uPreferredRecoveryStableSelectionCount >=
				uEffectiveHoldoffTarget)
			{
				uSelectedIndex = uPreferredIndex;
				pxContext->uPreferredRecoveryStableSelectionCount = 0U;
				pxContext->uPreferredRecoveryPendingPenaltySelections = 0U;
				pxContext->uLastSelectionWasFailover =
					(uint32_t)(uSelectedIndex != uPreviousIndex);
			}
			else
			{
				uSelectedIndex = pxContext->uActiveChannelIndex;
			}
		}
	}
	else if(uActiveIsAvailable != 0U)
	{
		uSelectedIndex = pxContext->uActiveChannelIndex;
		if((uPreferredIsAvailable == 0U) && (uHadHoldoffProgress != 0U))
		{
			pxContext->uPreferredRecoveryPendingPenaltySelections =
				pxContext->xConfig.uPreferredRecoveryFlapPenaltySelections;
		}
		pxContext->uPreferredRecoveryStableSelectionCount = 0U;
	}
	else if(uFindBestAvailableChannel(pxContext, &uSelectedIndex) != 0U)
	{
		pxContext->uPreferredRecoveryStableSelectionCount = 0U;
		if(uSelectedIndex == uPreferredIndex)
		{
			pxContext->uPreferredRecoveryPendingPenaltySelections = 0U;
		}
		pxContext->uLastSelectionWasFailover =
			(uint32_t)(uSelectedIndex != uPreviousIndex);
	}
	else
	{
		pxResult->eSelectedChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
		pxResult->uActiveChannelIndex = pxContext->uActiveChannelIndex;
		pxResult->uAvailableChannelCount = 0U;
		pxResult->uFailoverOccurred = 0U;
		pxResult->uTotalSwitchCount = pxContext->uTotalSwitchCount;
		if(pxContext->uUnavailableSelectionCount < UINT32_MAX)
		{
			pxContext->uUnavailableSelectionCount++;
		}
		pxResult->uUnavailableSelectionCount = pxContext->uUnavailableSelectionCount;
		pxContext->uPreferredRecoveryStableSelectionCount = 0U;
		vPopulateHoldoffTelemetry(pxContext, pxResult);
		return RSRX_CHANNEL_MANAGER_STATUS_UNAVAILABLE;
	}

	if((pxContext->uLastSelectionWasFailover != 0U) &&
		(pxContext->uTotalSwitchCount < UINT32_MAX))
	{
		pxContext->uTotalSwitchCount++;
	}

	pxContext->uActiveChannelIndex = uSelectedIndex;
	pxResult->eSelectedChannelId =
		pxContext->xConfig.axChannels[uSelectedIndex].eChannelId;
	pxResult->uActiveChannelIndex = uSelectedIndex;
	pxResult->uAvailableChannelCount = uCountAvailableChannels(pxContext);
	pxResult->uFailoverOccurred = pxContext->uLastSelectionWasFailover;
	pxResult->uTotalSwitchCount = pxContext->uTotalSwitchCount;
	pxResult->uUnavailableSelectionCount = pxContext->uUnavailableSelectionCount;
	vPopulateHoldoffTelemetry(pxContext, pxResult);

	return RSRX_CHANNEL_MANAGER_STATUS_OK;
}

rsrx_transport_channel_id_t rsrx_channel_manager_get_active_channel(
	const rsrx_channel_manager_context_t * pxContext)
{
	if((pxContext == (const rsrx_channel_manager_context_t *)0) ||
		(pxContext->uInitialized == 0U) ||
		(pxContext->uActiveChannelIndex >= pxContext->xConfig.uChannelCount))
	{
		return RSRX_TRANSPORT_CHANNEL_INVALID;
	}

	return pxContext->xConfig.axChannels[pxContext->uActiveChannelIndex].eChannelId;
}

rsrx_channel_manager_status_t rsrx_channel_manager_reset(
	rsrx_channel_manager_context_t * pxContext)
{
	if((pxContext == (rsrx_channel_manager_context_t *)0) ||
		(pxContext->uInitialized == 0U))
	{
		return RSRX_CHANNEL_MANAGER_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uActiveChannelIndex = pxContext->xConfig.uPreferredChannelIndex;
	pxContext->uLastSelectionWasFailover = 0U;
	pxContext->uPreferredRecoveryStableSelectionCount = 0U;
	pxContext->uPreferredRecoveryPendingPenaltySelections = 0U;

	return RSRX_CHANNEL_MANAGER_STATUS_OK;
}
