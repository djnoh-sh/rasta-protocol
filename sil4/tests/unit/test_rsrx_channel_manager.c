#include <stdio.h>
#include <stdlib.h>

#include "rsrx_channel_manager.h"

static void vAssertTrue(int iCondition, const char * pcMessage)
{
	if(iCondition == 0)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s\n", pcMessage);
		exit(EXIT_FAILURE);
	}
}

static rsrx_channel_manager_config_t xBuildConfig(void)
{
	rsrx_channel_manager_config_t xConfig;

	xConfig.eMode = RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY;
	xConfig.uChannelCount = 2U;
	xConfig.uPreferredChannelIndex = 0U;
	xConfig.uPreferredRecoveryHoldoffSelections = 0U;
	xConfig.axChannels[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xConfig.axChannels[0].uIsAvailable = 1U;
	xConfig.axChannels[0].uPriority = 0U;
	xConfig.axChannels[1].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xConfig.axChannels[1].uIsAvailable = 1U;
	xConfig.axChannels[1].uPriority = 1U;

	return xConfig;
}

static void vTestPreferredRecoveryHoldoff(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 2U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff mark primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff failover select");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff failover secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff switch count after failover");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff restore primary");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff first stable select");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff remains secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff no switch on first stable select");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff second stable select");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff switches to primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff switch count after recovery");
}

static void vTestPreferredRecoveryHysteresisResetMatrix(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 2U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "hysteresis matrix first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "hysteresis matrix first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix first hold");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "hysteresis matrix held secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "hysteresis matrix no switch on first hold");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "hysteresis matrix switch count held");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix flap down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "hysteresis matrix retained secondary after flap");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "hysteresis matrix no switch on flap refresh");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "hysteresis matrix switch count after flap");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix restored again");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix renewed hold");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "hysteresis matrix renewed held secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "hysteresis matrix no switch on renewed hold");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "hysteresis matrix renewed hold switch count");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "hysteresis matrix switched primary after renewed hold");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "hysteresis matrix switch reported after renewed hold");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "hysteresis matrix switch count after renewed recovery");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"hysteresis matrix no-op refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "hysteresis matrix retained primary on no-op refresh");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "hysteresis matrix no switch on no-op refresh");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "hysteresis matrix switch count stable on no-op refresh");
}

static void vTestPreferredRecoveryActiveLossBypassesHoldoff(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 2U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "active-loss matrix first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "active-loss matrix first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix first hold");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "active-loss matrix held secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "active-loss matrix no switch on hold");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "active-loss matrix held switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 1U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix secondary flap down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix preferred recovery bypass holdoff");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "active-loss matrix immediate preferred recovery");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "active-loss matrix switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "active-loss matrix switch count after bypass");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 1U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix secondary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"active-loss matrix no-op refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "active-loss matrix retained primary after restore");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "active-loss matrix no switch on refresh");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "active-loss matrix stable switch count");
}

static void vTestBypassReentersHoldoffOnNextCycle(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 2U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix primary down one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix failover one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "bypass-cycle matrix first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "bypass-cycle matrix first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix primary restored one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "bypass-cycle matrix held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "bypass-cycle matrix no switch on hold one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 1U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix secondary down one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix bypass one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "bypass-cycle matrix recovered primary one");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "bypass-cycle matrix switch reported one");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "bypass-cycle matrix switch count after bypass");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 1U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix secondary restored one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix refresh one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "bypass-cycle matrix retained primary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "bypass-cycle matrix no switch on refresh one");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "bypass-cycle matrix stable switch count one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix primary down two");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix failover two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "bypass-cycle matrix second secondary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "bypass-cycle matrix switch reported two");
	vAssertTrue(xResult.uTotalSwitchCount == 3U, "bypass-cycle matrix switch count after second failover");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix primary restored two");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix renewed hold");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "bypass-cycle matrix held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "bypass-cycle matrix no switch on renewed hold");
	vAssertTrue(xResult.uTotalSwitchCount == 3U, "bypass-cycle matrix switch count held two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"bypass-cycle matrix renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "bypass-cycle matrix recovered primary two");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "bypass-cycle matrix switch reported on renewed recovery");
	vAssertTrue(xResult.uTotalSwitchCount == 4U, "bypass-cycle matrix switch count after renewed recovery");
}

static void vTestFlapResetBypassReentersHoldoffMatrix(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 2U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix primary down one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix failover one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "flap-bypass-cycle matrix first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "flap-bypass-cycle matrix first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix primary restored one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "flap-bypass-cycle matrix held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "flap-bypass-cycle matrix no switch on hold one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix primary flap down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "flap-bypass-cycle matrix retained secondary after flap");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "flap-bypass-cycle matrix no switch on flap refresh");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "flap-bypass-cycle matrix switch count after flap");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix primary restored again");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix renewed hold");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "flap-bypass-cycle matrix renewed held secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "flap-bypass-cycle matrix no switch on renewed hold");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "flap-bypass-cycle matrix renewed hold switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 1U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix secondary down one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix bypass one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "flap-bypass-cycle matrix recovered primary one");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "flap-bypass-cycle matrix switch reported one");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "flap-bypass-cycle matrix switch count after bypass");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 1U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix secondary restored one");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix refresh one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "flap-bypass-cycle matrix retained primary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "flap-bypass-cycle matrix no switch on refresh one");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "flap-bypass-cycle matrix stable switch count one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix primary down two");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix failover two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "flap-bypass-cycle matrix second secondary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "flap-bypass-cycle matrix switch reported two");
	vAssertTrue(xResult.uTotalSwitchCount == 3U, "flap-bypass-cycle matrix switch count after second failover");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix primary restored two");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "flap-bypass-cycle matrix held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "flap-bypass-cycle matrix no switch on renewed hold two");
	vAssertTrue(xResult.uTotalSwitchCount == 3U, "flap-bypass-cycle matrix switch count held two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"flap-bypass-cycle matrix renewed recovery two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "flap-bypass-cycle matrix recovered primary two");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "flap-bypass-cycle matrix switch reported on renewed recovery two");
	vAssertTrue(xResult.uTotalSwitchCount == 4U, "flap-bypass-cycle matrix switch count after renewed recovery two");
}

int main(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"channel manager init");
	vAssertTrue(
		rsrx_channel_manager_get_active_channel(&xContext) == RSRX_TRANSPORT_CHANNEL_PRIMARY,
		"active channel after init");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"select primary");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "selected primary");
	vAssertTrue(xResult.uAvailableChannelCount == 2U, "available channel count");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "no failover on first selection");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"mark primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"select failover channel");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "selected secondary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "failover detected");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "switch count after failover");
	vAssertTrue(
		rsrx_channel_manager_get_active_channel(&xContext) == RSRX_TRANSPORT_CHANNEL_SECONDARY,
		"active secondary after failover");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 1U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"mark secondary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_UNAVAILABLE,
		"no available channel");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_INVALID, "invalid channel on unavailable");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"select preferred after restore");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "selected primary after restore");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "channel switch reported on preferred recovery");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "switch count after preferred recovery");
	vAssertTrue(
		rsrx_channel_manager_get_active_channel(&xContext) == RSRX_TRANSPORT_CHANNEL_PRIMARY,
		"active primary after restore");
	vAssertTrue(
		rsrx_channel_manager_reset(&xContext) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"channel manager reset");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"select primary after reset");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "selected primary after reset");

	vTestPreferredRecoveryHoldoff();
	vTestPreferredRecoveryHysteresisResetMatrix();
	vTestPreferredRecoveryActiveLossBypassesHoldoff();
	vTestBypassReentersHoldoffOnNextCycle();
	vTestFlapResetBypassReentersHoldoffMatrix();

	(void)printf("rsrx_channel_manager_test: all tests passed\n");
	return EXIT_SUCCESS;
}
