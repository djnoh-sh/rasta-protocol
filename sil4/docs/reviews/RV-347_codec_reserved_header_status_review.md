# RV-347 Codec Reserved Header Status Review

- Review ID: `RV-347`
- Date: `2026-05-07`
- Scope: reserved-header tamper-specific codec status.

## Findings

- `TC-CODEC-007` and `TC-CODEC-019` now expect `RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO` instead of generic `DECODE_ERROR`.
- The codec still rejects every currently reserved header offset, but the failure reason is now distinct from generic malformed-frame failures.
- This narrows the R-006 status-taxonomy residual without claiming MAC/timestamp/PDU parity.

## Residual

- CRC mismatch/truncation and reserved-header tamper now have specific codec statuses.
- Additional stale/tamper taxonomy and actual MAC/timestamp/PDU implementation remain future codec-security growth.
