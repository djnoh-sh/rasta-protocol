# Review Record - RV-505 Unsequenced Connect Request Integration Ordering

## Scope

- `TC-INT-213`
- `R-001 Protocol sequencing`
- Session-supervisor integration boundary

## Findings

- The integration harness now exercises inbound `CONNECT_REQUEST` against an initialized session with intentionally arbitrary `sequence=123` and `confirmation=45`.
- The frame is treated as an unsequenced `VALID_INBOUND_CONNECT` event rather than a protocol ordering error.
- The session enters `CONNECTING` with accepted session status and `RSRX_REASON_INBOUND_CONNECT_ACCEPTED`.
- No application data callback is emitted for the connect-request frame.

## Residual

- This adds session-supervisor parity for the unsequenced `CONNECT_REQUEST` family representative.
- Additional unsequenced-family integration cases should be added only if they expose different state-machine or codec behavior.

## Conclusion

Accepted as protocol sequencing integration evidence that unsequenced inbound connect-request messages do not pollute sequence/confirmation ordering state.
