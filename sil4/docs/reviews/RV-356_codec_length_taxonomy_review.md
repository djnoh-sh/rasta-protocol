# RV-356 Codec Length Taxonomy Review

- Review ID: `RV-356`
- Date: `2026-05-08`
- Scope: declared/actual payload length mismatch taxonomy.

## Findings

- `TC-CODEC-009` now expects `RSRX_CODEC_STATUS_TRAILING_BYTES` when the actual frame is longer than the declared payload length.
- `TC-CODEC-010` now expects `RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD` when the actual frame is shorter than the declared payload length.
- The default codec direct decode path no longer collapses trailing and truncated payload length failures into `RSRX_CODEC_STATUS_LENGTH_MISMATCH`.
- `RSRX_CODEC_STATUS_LENGTH_MISMATCH` remains in the public enum as a compatibility status for alternate/custom codec ports that cannot provide the narrower length-failure cause.

## Residual

- Declared/actual length overrun and underrun are now separable for current default codec negative vectors.
- Actual MAC/timestamp/PDU parity and additional stale/tamper-specific vectors remain future codec-security growth.
