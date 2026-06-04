# Review Record - RV-437 Channel Manager Snapshot Supervisor Refresh

## Scope

- `rsrx_channel_manager_snapshot_t`
- `rsrx_session_copy_channel_manager_snapshot`
- `vRefreshChannelSwitchTelemetry`
- `TC-API-024`
- `TC-SUP-079`
- `R-009 API concurrency and reset quiescence`

## Review Result

- The public API now exposes a caller-owned channel-manager snapshot containing active/preferred channel IDs, available-channel count, switch counters, unavailable-selection count, and preferred-recovery holdoff/penalty telemetry.
- The snapshot API clears caller output before validation and clears it again on critical-section exit failure, preventing stale caller-owned data on rejected paths.
- Snapshot capture is performed inside the existing session critical-section seam, keeping direct channel-manager state reads inside the API-owned guard boundary.
- Transport supervisor switch/holdoff report refresh now consumes `rsrx_session_copy_channel_manager_snapshot` instead of directly reading channel-manager internals for report fields.
- Existing supervisor switch/holdoff tests continue to exercise the success path, and `TC-API-024` covers snapshot success plus enter-failure output clearing.

## Residual

- `RV-437` does not claim SafeRTOS multi-task/ISR safety by itself; target critical-section binding, callback reentrancy/deferred-callback policy, and target integration/fault-injection logs remain required.
- Other supervisor/session interactions that may be used from separate tasks remain subject to the same target concurrency boundary review.
- The active-channel getter remains available for existing single-field policy decisions; target integration policy should prefer caller-owned snapshot APIs for multi-field report refreshes.

## Disposition

Accepted as a portable host reduction of `R-009` residual for supervisor switch/holdoff report refresh.
