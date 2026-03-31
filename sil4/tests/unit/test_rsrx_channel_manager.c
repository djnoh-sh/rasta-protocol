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

static void vTestPreferredRecoveryHoldoffThresholdThree(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 3U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-3 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 primary restored");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-3 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-3 stable switch count one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-3 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-3 stable switch count two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-3 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-3 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-3 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdFour(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 4U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-4 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 primary restored");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-4 stable switch count one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-4 stable switch count two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-4 stable switch count three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-4 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-4 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-4 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdFive(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 5U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-5 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 primary restored");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-5 stable switch count one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-5 stable switch count two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-5 stable switch count three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-5 stable switch count four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-5 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-5 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-5 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdSix(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 6U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 primary restored");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 stable switch count one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 stable switch count two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 stable switch count three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 stable switch count four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 no switch five");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 stable switch count five");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-6 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-6 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-6 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdSeven(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 7U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 primary restored");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 stable switch count one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 stable switch count two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 stable switch count three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 stable switch count four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 no switch five");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 stable switch count five");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 no switch six");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 stable switch count six");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-7 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-7 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-7 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdEight(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 8U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 primary restored");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 stable switch count one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 stable switch count two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 stable switch count three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 stable switch count four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 no switch five");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 stable switch count five");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 no switch six");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 stable switch count six");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 no switch seven");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 stable switch count seven");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-8 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-8 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-8 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdNine(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 9U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 primary restored");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count one");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count two");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count three");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count four");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch five");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count five");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch six");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count six");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch seven");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count seven");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 no switch eight");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 stable switch count eight");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-9 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-9 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-9 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdTen(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 10U;

	vAssertTrue(rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 primary down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 primary restored");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count one");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count two");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count three");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count four");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch five");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count five");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch six");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count six");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch seven");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count seven");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch eight");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count eight");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 hold nine");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 held secondary nine");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 no switch nine");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 stable switch count nine");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-10 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-10 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-10 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdEleven(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 11U;

	vAssertTrue(rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 primary down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 primary restored");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count one");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count two");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count three");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count four");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch five");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count five");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch six");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count six");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch seven");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count seven");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch eight");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count eight");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold nine");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary nine");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch nine");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count nine");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 hold ten");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 held secondary ten");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 no switch ten");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 stable switch count ten");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-11 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-11 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-11 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdTwelve(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 12U;

	vAssertTrue(rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 primary down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 failover switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 primary restored");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count one");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch two");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count two");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch three");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count three");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch four");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count four");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch five");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count five");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch six");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count six");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch seven");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count seven");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch eight");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count eight");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold nine");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary nine");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch nine");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count nine");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold ten");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary ten");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch ten");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count ten");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 hold eleven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 held secondary eleven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 no switch eleven");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 stable switch count eleven");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-12 recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-12 switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-12 recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdThreeFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 3U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-3 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-3 flap no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-3 flap stable switch count one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap reset down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-3 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-3 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap primary restored again");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-3 flap no switch renewed one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-3 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-3 flap no switch renewed two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-3 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-3 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-3 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-3 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdFourFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 4U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-4 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 flap no switch one");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-4 flap stable switch count one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap reset down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-4 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap primary restored again");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 flap no switch renewed one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 flap no switch renewed two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-4 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-4 flap no switch renewed three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-4 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-4 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-4 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-4 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdFiveFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 5U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-5 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap reset down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-5 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap primary restored again");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 flap no switch renewed one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 flap no switch renewed two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 flap no switch renewed three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-5 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-5 flap no switch renewed four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-5 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-5 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-5 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-5 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdSixFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 6U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap reset down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-6 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap primary restored again");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 flap no switch renewed one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 flap no switch renewed two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 flap no switch renewed three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 flap no switch renewed four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap renewed hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-6 flap renewed held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-6 flap no switch renewed five");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-6 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-6 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-6 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-6 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdSevenFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 7U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap reset down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-7 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap primary restored again");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch renewed one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch renewed two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch renewed three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch renewed four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap renewed hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap renewed held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch renewed five");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap renewed hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-7 flap renewed held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-7 flap no switch renewed six");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-7 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-7 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-7 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-7 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdEightFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 8U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap reset down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-8 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap primary restored again");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch renewed one");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch renewed two");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch renewed three");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch renewed four");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap renewed held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch renewed five");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap renewed held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch renewed six");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-8 flap renewed held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-8 flap no switch renewed seven");

	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-8 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-8 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-8 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-8 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdNineFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 9U;

	vAssertTrue(
		rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap primary down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap primary restored");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap reset down");
	vAssertTrue(
		rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-9 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(
		rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK,
		"holdoff-9 flap primary restored again");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed one");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed two");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed three");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed four");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed five");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed six");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed seven");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-9 flap renewed held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-9 flap no switch renewed eight");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-9 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-9 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-9 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-9 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdTenFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 10U;

	vAssertTrue(rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap primary down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap primary restored");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap reset down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-10 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap primary restored again");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed one");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed two");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed three");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed four");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed five");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed six");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed seven");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed eight");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed hold nine");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-10 flap renewed held secondary nine");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-10 flap no switch renewed nine");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-10 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-10 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-10 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-10 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdElevenFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 11U;

	vAssertTrue(rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap primary down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap primary restored");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap reset down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-11 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap primary restored again");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed one");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed two");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed three");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed four");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed five");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed six");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed seven");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed eight");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold nine");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary nine");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed nine");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed hold ten");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-11 flap renewed held secondary ten");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-11 flap no switch renewed ten");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-11 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-11 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-11 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-11 flap recovery switch count");
}

static void vTestPreferredRecoveryHoldoffThresholdTwelveFlapReset(void)
{
	rsrx_channel_manager_context_t xContext;
	rsrx_channel_manager_config_t xConfig;
	rsrx_channel_selection_result_t xResult;
	rsrx_transport_channel_state_t xState;

	xConfig = xBuildConfig();
	xConfig.uPreferredRecoveryHoldoffSelections = 12U;

	vAssertTrue(rsrx_channel_manager_init(&xContext, &xConfig) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap init");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap primary down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap failover");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap first secondary");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 flap first switch count");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap primary restored");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch one");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 0U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap reset down");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap refresh");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap retained secondary");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch on reset");
	vAssertTrue(xResult.uTotalSwitchCount == 1U, "holdoff-12 flap switch count after reset");

	xState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xState.uIsAvailable = 1U;
	vAssertTrue(rsrx_channel_manager_update_channel(&xContext, 0U, &xState) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap primary restored again");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold one");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary one");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed one");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold two");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary two");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed two");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold three");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary three");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed three");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold four");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary four");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed four");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold five");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary five");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed five");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold six");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary six");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed six");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold seven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary seven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed seven");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold eight");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary eight");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed eight");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold nine");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary nine");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed nine");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold ten");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary ten");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed ten");
	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed hold eleven");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY, "holdoff-12 flap renewed held secondary eleven");
	vAssertTrue(xResult.uFailoverOccurred == 0U, "holdoff-12 flap no switch renewed eleven");

	vAssertTrue(rsrx_channel_manager_select_channel(&xContext, &xResult) == RSRX_CHANNEL_MANAGER_STATUS_OK, "holdoff-12 flap renewed recovery");
	vAssertTrue(xResult.eSelectedChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "holdoff-12 flap renewed recovered primary");
	vAssertTrue(xResult.uFailoverOccurred == 1U, "holdoff-12 flap switch reported");
	vAssertTrue(xResult.uTotalSwitchCount == 2U, "holdoff-12 flap recovery switch count");
}

static void vTestPreferredRecoveryThresholdCloseoutMatrix(void)
{
	vTestPreferredRecoveryHoldoffThresholdThree();
	vTestPreferredRecoveryHoldoffThresholdFour();
	vTestPreferredRecoveryHoldoffThresholdFive();
	vTestPreferredRecoveryHoldoffThresholdSix();
	vTestPreferredRecoveryHoldoffThresholdSeven();
	vTestPreferredRecoveryHoldoffThresholdEight();
	vTestPreferredRecoveryHoldoffThresholdNine();
	vTestPreferredRecoveryHoldoffThresholdTen();
	vTestPreferredRecoveryHoldoffThresholdEleven();
	vTestPreferredRecoveryHoldoffThresholdTwelve();
	vTestPreferredRecoveryHoldoffThresholdThreeFlapReset();
	vTestPreferredRecoveryHoldoffThresholdFourFlapReset();
	vTestPreferredRecoveryHoldoffThresholdFiveFlapReset();
	vTestPreferredRecoveryHoldoffThresholdSixFlapReset();
	vTestPreferredRecoveryHoldoffThresholdSevenFlapReset();
	vTestPreferredRecoveryHoldoffThresholdEightFlapReset();
	vTestPreferredRecoveryHoldoffThresholdNineFlapReset();
	vTestPreferredRecoveryHoldoffThresholdTenFlapReset();
	vTestPreferredRecoveryHoldoffThresholdElevenFlapReset();
	vTestPreferredRecoveryHoldoffThresholdTwelveFlapReset();
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

static void vTestPreferredRecoveryHysteresisCloseoutMatrix(void)
{
	vTestPreferredRecoveryHysteresisResetMatrix();
	vTestPreferredRecoveryActiveLossBypassesHoldoff();
	vTestBypassReentersHoldoffOnNextCycle();
	vTestFlapResetBypassReentersHoldoffMatrix();
}

static void vTestPreferredRecoveryStabilityEnvelopeCloseoutMatrix(void)
{
	vTestPreferredRecoveryHysteresisCloseoutMatrix();
	vTestPreferredRecoveryThresholdCloseoutMatrix();
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
	vTestPreferredRecoveryHoldoffThresholdThree();
	vTestPreferredRecoveryHoldoffThresholdFour();
	vTestPreferredRecoveryHoldoffThresholdFive();
	vTestPreferredRecoveryHoldoffThresholdSix();
	vTestPreferredRecoveryHoldoffThresholdSeven();
	vTestPreferredRecoveryHoldoffThresholdEight();
	vTestPreferredRecoveryHoldoffThresholdNine();
	vTestPreferredRecoveryHoldoffThresholdTen();
	vTestPreferredRecoveryHoldoffThresholdEleven();
	vTestPreferredRecoveryHoldoffThresholdTwelve();
	vTestPreferredRecoveryHoldoffThresholdThreeFlapReset();
	vTestPreferredRecoveryHoldoffThresholdFourFlapReset();
	vTestPreferredRecoveryHoldoffThresholdFiveFlapReset();
	vTestPreferredRecoveryHoldoffThresholdSixFlapReset();
	vTestPreferredRecoveryHoldoffThresholdSevenFlapReset();
	vTestPreferredRecoveryHoldoffThresholdEightFlapReset();
	vTestPreferredRecoveryHoldoffThresholdNineFlapReset();
	vTestPreferredRecoveryHoldoffThresholdTenFlapReset();
	vTestPreferredRecoveryHoldoffThresholdElevenFlapReset();
	vTestPreferredRecoveryHoldoffThresholdTwelveFlapReset();
	vTestPreferredRecoveryThresholdCloseoutMatrix();
	vTestPreferredRecoveryHysteresisResetMatrix();
	vTestPreferredRecoveryActiveLossBypassesHoldoff();
	vTestBypassReentersHoldoffOnNextCycle();
	vTestFlapResetBypassReentersHoldoffMatrix();
	vTestPreferredRecoveryHysteresisCloseoutMatrix();
	vTestPreferredRecoveryStabilityEnvelopeCloseoutMatrix();

	(void)printf("rsrx_channel_manager_test: all tests passed\n");
	return EXIT_SUCCESS;
}
