# Review Record - RV-278 Invalid Confirmation Record Guard

## Scope

- Prevent invalid remote confirmation values from mutating protocol context state during direct inbound record.
- Extend `TC-PC-023` to cover sent-high-watermark overflow and regressing confirmation record attempts.

## Review Notes

- `rsrx_protocol_context_record_inbound_message` now applies the same confirmation monotonicity/high-watermark guard used by inbound resolution.
- Invalid sequenced records return `REJECTED`.
- Rejected record attempts leave last received sequence, tx confirmation, and remote confirmation unchanged.
- Unsequenced record pass-through behavior remains unchanged.

## Evidence

- `vTestInvalidConfirmationRecordRejected`
- `TC-PC-023`

## Decision

- Accepted.

## Residual

- Future `R-001` work should focus on richer confirm/retransmission ordering variants or additional session-supervisor parity rather than record-stage invalid-confirmation contamination.
