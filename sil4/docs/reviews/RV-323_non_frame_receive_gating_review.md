# RV-323 Non-Frame Receive Gating Review

## Scope

- Review ID: `RV-323`
- Scope: `non-frame receive no-frame gating integration`
- Date: 2026-05-06

## Findings

1. `TC-INT-207` drives a fake-transport integration path with a valid payload attached to a non-`FRAME_RECEIVED` receive event.
2. The supervisor reports `RSRX_SUPERVISOR_STATUS_NO_FRAME` and preserves `RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE` without forwarding the event to the codec or session/application layer.
3. The test verifies the established session remains established, the processed-frame count is not advanced by the gated event, and the non-frame event type is preserved in the supervisor report.

## Disposition

- Pass.
- Supervisor receive-path event-type gating now has integration-level evidence before codec/session handoff.
