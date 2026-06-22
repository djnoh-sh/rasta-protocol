# RV-353 Codec Invalid Channel Status Review

- Review ID: `RV-353`
- Date: `2026-05-08`
- Scope: invalid transport channel codec status.

## Findings

- `TC-CODEC-023` now expects `RSRX_CODEC_STATUS_INVALID_CHANNEL` when codec decode is directly called with an invalid transport channel id.
- `TC-INT-206` now verifies the supervisor preserves that codec status while still returning `DECODE_FAILED` and retaining session state.
- This keeps transport-origin misuse distinct from generic malformed-frame decode errors.

## Residual

- Invalid-channel diagnostics are no longer collapsed into `DECODE_ERROR`.
- Actual MAC/timestamp/PDU implementation remains future codec-security growth.
