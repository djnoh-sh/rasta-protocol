# RV-348 Codec Length Mismatch Status Review

- Review ID: `RV-348`
- Date: `2026-05-07`
- Scope: declared/actual payload-length mismatch codec status.

## Findings

- `TC-CODEC-009` now expects `RSRX_CODEC_STATUS_LENGTH_MISMATCH` for trailing bytes after a declared zero-length payload.
- `TC-CODEC-010` now expects the same status for a declared payload that is shorter than the actual frame.
- Oversized declared payload remains a bounded-buffer decode guard and is still handled separately from declared/actual mismatch.

## Residual

- Length-mismatch diagnostics are no longer collapsed into generic `DECODE_ERROR`.
- Additional stale/tamper taxonomy and actual MAC/timestamp/PDU implementation remain future codec-security growth.
