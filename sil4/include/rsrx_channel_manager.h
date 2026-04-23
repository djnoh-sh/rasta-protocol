#ifndef RSRX_CHANNEL_MANAGER_H
#define RSRX_CHANNEL_MANAGER_H

#include <stdint.h>

#include "rsrx_transport.h"

#define D_RSRX_CHANNEL_MANAGER_MAX_CHANNELS (2U)

typedef enum
{
	RSRX_CHANNEL_MANAGER_STATUS_OK = 0,
	RSRX_CHANNEL_MANAGER_STATUS_INVALID_ARGUMENT,
	RSRX_CHANNEL_MANAGER_STATUS_UNAVAILABLE
} rsrx_channel_manager_status_t;

typedef enum
{
	RSRX_REDUNDANCY_MODE_INVALID = 0,
	RSRX_REDUNDANCY_MODE_SINGLE,
	RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY
} rsrx_redundancy_mode_t;

typedef struct
{
	rsrx_transport_channel_id_t eChannelId;
	uint32_t uIsAvailable;
	uint32_t uPriority;
} rsrx_channel_descriptor_t;

typedef struct
{
	rsrx_redundancy_mode_t eMode;
	rsrx_channel_descriptor_t axChannels[D_RSRX_CHANNEL_MANAGER_MAX_CHANNELS];
	uint32_t uChannelCount;
	uint32_t uPreferredChannelIndex;
	uint32_t uPreferredRecoveryHoldoffSelections;
} rsrx_channel_manager_config_t;

typedef struct
{
	rsrx_transport_channel_id_t eSelectedChannelId;
	uint32_t uActiveChannelIndex;
	uint32_t uAvailableChannelCount;
	uint32_t uFailoverOccurred;
	uint32_t uTotalSwitchCount;
	uint32_t uUnavailableSelectionCount;
	uint32_t uPreferredRecoveryHoldoffActive;
	uint32_t uPreferredRecoveryHoldoffProgressCount;
	uint32_t uPreferredRecoveryHoldoffTargetCount;
	uint32_t uPreferredRecoveryHoldoffRemainingCount;
} rsrx_channel_selection_result_t;

typedef struct
{
	rsrx_channel_manager_config_t xConfig;
	uint32_t uActiveChannelIndex;
	uint32_t uLastSelectionWasFailover;
	uint32_t uPreferredRecoveryStableSelectionCount;
	uint32_t uTotalSwitchCount;
	uint32_t uUnavailableSelectionCount;
	uint32_t uInitialized;
} rsrx_channel_manager_context_t;

rsrx_channel_manager_status_t rsrx_channel_manager_init(
	rsrx_channel_manager_context_t * pxContext,
	const rsrx_channel_manager_config_t * pxConfig);

rsrx_channel_manager_status_t rsrx_channel_manager_update_channel(
	rsrx_channel_manager_context_t * pxContext,
	uint32_t uChannelIndex,
	const rsrx_transport_channel_state_t * pxState);

rsrx_channel_manager_status_t rsrx_channel_manager_select_channel(
	rsrx_channel_manager_context_t * pxContext,
	rsrx_channel_selection_result_t * pxResult);

rsrx_transport_channel_id_t rsrx_channel_manager_get_active_channel(
	const rsrx_channel_manager_context_t * pxContext);

rsrx_channel_manager_status_t rsrx_channel_manager_reset(
	rsrx_channel_manager_context_t * pxContext);

#endif
