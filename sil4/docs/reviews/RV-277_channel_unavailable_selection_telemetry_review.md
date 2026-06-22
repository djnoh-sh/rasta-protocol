# Review Record - RV-277 Channel Unavailable Selection Telemetry

## Scope

- Preserve cumulative all-channel-unavailable observations in channel manager selection telemetry.
- Expose the same counter through the transport supervisor report for channel-gated receive paths.

## Review Notes

- `uUnavailableSelectionCount` increments when `rsrx_channel_manager_select_channel` returns `UNAVAILABLE`.
- The counter is cumulative and is retained across later recovery and reset operations, matching existing switch-count audit behavior.
- `uChannelUnavailableSelectionCount` mirrors the channel manager counter in supervisor reports after channel refresh/query.
- This change is redundancy observability refinement, not numeric holdoff or backlog growth.

## Evidence

- `TC-CHM-003`
- `TC-SUP-008`
- `TC-SUP-026`

## Decision

- Accepted.

## Residual

- Future `R-003` work should focus on new redundancy policy variants or broader long-run stability semantics rather than all-channel-unavailable observation visibility.
