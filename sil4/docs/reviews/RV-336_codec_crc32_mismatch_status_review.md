# RV-336 Codec CRC32 Mismatch Status Review

- Review ID: `RV-336`
- Date: `2026-05-07`
- Scope: CRC32 wire decode mismatch reporting and related unit/spec/roadmap traceability.

## Findings

- `rsrx_codec_decode_frame_with_crc32()` now reports `RSRX_CODEC_STATUS_CRC_MISMATCH` when the appended checksum does not match the decoded payload frame.
- `TC-CODEC-028` separates checksum/tamper-style negative evidence from generic malformed-frame `DECODE_ERROR` coverage.
- The default skeleton codec path remains unchanged; the new status is only returned by the optional CRC32 wire decode path.

## Residual

- This does not implement MAC or timestamp semantics.
- Supervisor/reporting policy still treats non-OK codec results as decode failure; richer end-to-end reporting remains future codec-security policy growth.
