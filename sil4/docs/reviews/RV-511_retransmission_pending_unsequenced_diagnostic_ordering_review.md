# Review Record - RV-511 Retransmission-Pending Unsequenced Diagnostic Ordering

## Scope

- `TC-INT-215`
- `R-001 Protocol sequencing`
- Integration session/supervisor receive path

## Findings

- The integration harness now covers an inbound `DIAGNOSTIC` frame while the session is already in `RETRANSMISSION_PENDING`.
- The flow first establishes the session, records valid data, and injects a sequenced data gap to enter retransmission pending.
- The diagnostic frame uses gap-like `sequence=99` and `confirmation=77` values.
- The supervisor/session path treats the decoded diagnostic as `RSRX_EVENT_PROTOCOL_ERROR`, not recovery success or another sequence-gap event.
- Because `RETRANSMISSION_PENDING` has no dedicated protocol-error rule, the state machine enters `SAFE_DISCONNECT` through the rejected conservative fail-safe path and does not deliver additional application data.

## Residual

- Current baseline still treats `CONNECT_REQUEST`, `DISCONNECT`, and `DIAGNOSTIC` as the unsequenced inbound family.
- Future stricter sequencing rules remain selected protocol-policy work and must not silently alter the current baseline.

## Conclusion

Accepted as integration evidence that unsequenced diagnostic pass-through remains stable even during retransmission pending.
