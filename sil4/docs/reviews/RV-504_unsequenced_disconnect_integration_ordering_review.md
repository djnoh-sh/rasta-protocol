# Review Record - RV-504 Unsequenced Disconnect Integration Ordering

## Scope

- `TC-INT-212`
- `R-001 Protocol sequencing`
- Session-supervisor integration boundary

## Findings

- The integration harness now exercises inbound `DISCONNECT` after a completed handshake with intentionally arbitrary `sequence=99` and `confirmation=77`.
- The frame is treated as an unsequenced `DISCONNECT_REQUEST` event rather than a protocol ordering error.
- The session reaches `SAFE_DISCONNECT` with accepted session status and `RSRX_REASON_DISCONNECT_REQUESTED`.
- No application data callback is emitted for the disconnect frame.

## Residual

- This adds session-supervisor parity for the unsequenced `DISCONNECT` family representative.
- Additional unsequenced-family integration cases should be added only if they expose different state-machine or codec behavior.

## Conclusion

Accepted as protocol sequencing integration evidence that unsequenced inbound disconnect messages do not pollute sequence/confirmation ordering state.
