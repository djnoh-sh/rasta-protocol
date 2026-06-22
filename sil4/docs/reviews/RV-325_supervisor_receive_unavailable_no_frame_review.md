# RV-325 Supervisor Receive-Unavailable No-Frame Review

## Scope

- Review ID: `RV-325`
- Scope: `supervisor receive-unavailable no-frame telemetry`
- Date: 2026-05-06

## Findings

1. `TC-SUP-008` now asserts the receive-unavailable path returns `RSRX_SUPERVISOR_STATUS_NO_FRAME` without calling the codec or processing a frame.
2. The test verifies `RSRX_SUPERVISOR_DECISION_NO_FRAME_AVAILABLE`, `RSRX_SUPERVISOR_DECISION_CLASS_IGNORED`, ignored-decision accumulation, `RSRX_TRANSPORT_STATUS_UNAVAILABLE`, and receive-error stage `NONE`.
3. The added assertions make the idle receive path telemetry consistent with the newer non-frame no-frame gating evidence.

## Disposition

- Pass.
- Receive-unavailable no-frame handling now has explicit unit-level runtime feedback evidence.
