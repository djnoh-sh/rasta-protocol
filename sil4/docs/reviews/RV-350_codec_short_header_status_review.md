# RV-350 Codec Short Header Status Review

- Review ID: `RV-350`
- Date: `2026-05-08`
- Scope: short-header codec status.

## Findings

- `TC-CODEC-017` now expects `RSRX_CODEC_STATUS_SHORT_HEADER` for frames shorter than `D_RSRX_CODEC_HEADER_BYTES`.
- The status remains distinct from null-argument handling and from declared/actual payload-length mismatch.
- This narrows the codec status taxonomy residual for malformed wire-format boundaries.

## Residual

- Short-header diagnostics are no longer collapsed into generic `DECODE_ERROR`.
- Transport-origin decode rejections and actual MAC/timestamp/PDU implementation remain future codec-security growth.
