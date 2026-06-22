# RV-326 Supervisor Channel-Gated Receive Review

## Scope

- Review ID: `RV-326`
- Scope: `supervisor channel-gated receive telemetry`
- Date: 2026-05-06

## Findings

1. `TC-SUP-008` now asserts the unavailable-channel receive path returns `RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN` before calling receive or codec.
2. The test verifies `RSRX_SUPERVISOR_DECISION_CHANNEL_GATED_DOWN`, error decision class/count, available-channel count `0`, unavailable-selection count `1`, query status `CHANNEL_DOWN`, and receive-error stage `NONE`.
3. The added assertions make the channel-gated receive path telemetry consistent with the receive-unavailable no-frame and non-frame no-frame receive evidence.

## Disposition

- Pass.
- Channel-gated receive handling now has explicit unit-level runtime feedback evidence before decode/session handoff.
