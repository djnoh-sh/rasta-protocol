# Review Record - RV-510 Unsequenced Diagnostic Integration Ordering

## Scope

- `TC-INT-214`
- `R-001 Protocol sequencing`
- `R-007 V&V accepted follow-up`
- Integration session/supervisor receive path

## Findings

- The integration harness now covers an inbound `DIAGNOSTIC` frame after the handshake is established.
- The frame uses gap-like `sequence=99` and `confirmation=77` values to distinguish pass-through behavior from sequenced-family gap detection.
- The supervisor/session path treats the decoded diagnostic as `RSRX_EVENT_PROTOCOL_ERROR`, not `RSRX_EVENT_SEQUENCE_GAP_DETECTED`.
- The resulting transition reaches `SAFE_DISCONNECT` through the rejected protocol-error reason and does not deliver application data.

## Residual

- Current baseline still treats `CONNECT_REQUEST`, `DISCONNECT`, and `DIAGNOSTIC` as the unsequenced inbound family.
- Any future stricter sequencing interpretation remains a selected protocol-policy change, not a silent change to the current baseline.

## Conclusion

Accepted as integration evidence that the unsequenced diagnostic path preserves the current ordering-pass-through contract.
