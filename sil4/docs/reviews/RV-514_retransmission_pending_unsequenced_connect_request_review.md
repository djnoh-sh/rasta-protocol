# RV-514 Retransmission-Pending Unsequenced Connect-Request Review

## Scope

- Requirement area: `R-001 Protocol sequencing`
- Test case: `TC-INT-217`
- Files reviewed:
  - `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
  - `sil4/docs/verification/integration_harness_test_spec_draft.md`
  - `sil4/docs/roadmap_status.md`

## Review Questions

1. Does a `CONNECT_REQUEST` received while retransmission is pending bypass sequence/confirmation recovery ordering?
2. Is the current state-machine result explicit when `VALID_INBOUND_CONNECT` is not a defined retransmission-pending transition?
3. Are application and lifecycle callback effects deterministic?

## Findings

1. `TC-INT-217` drives handshake, valid data, and a higher-sequence gap into `RETRANSMISSION_PENDING`, then injects `CONNECT_REQUEST` with `sequence=123` and `confirmation=45`.
2. The supervisor reports `RSRX_EVENT_VALID_INBOUND_CONNECT`, proving the unsequenced message was not converted into a recovery success, repeated gap, or protocol-ordering error.
3. Because the current state machine intentionally lacks `RETRANSMISSION_PENDING + VALID_INBOUND_CONNECT`, the transition is rejected through conservative fail-safe with `RSRX_REASON_CONSERVATIVE_FAILSAFE`.
4. The test confirms no additional application callback is emitted and exactly one lifecycle callback is produced for the terminal fail-safe transition.

## Residual

- This closes the current retransmission-pending `CONNECT_REQUEST` ordering-pass-through evidence gap.
- Broader residual remains richer future retransmission message families and any later policy decision to explicitly accept inbound connect requests while retransmission is pending.

## Decision

Accepted as additional protocol sequencing evidence for retransmission-pending unsequenced message handling.
