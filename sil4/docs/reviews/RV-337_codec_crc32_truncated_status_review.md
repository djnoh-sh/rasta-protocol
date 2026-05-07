# RV-337 Codec CRC32 Truncated Status Review

- Review ID: `RV-337`
- Date: `2026-05-07`
- Scope: CRC32 wire decode checksum-field truncation reporting and related unit/spec/roadmap traceability.

## Findings

- `rsrx_codec_decode_frame_with_crc32()` now reports `RSRX_CODEC_STATUS_CRC_TRUNCATED` when a frame is too short to contain the required CRC32 field.
- `TC-CODEC-029` separates missing/truncated checksum-field evidence from generic malformed-frame `DECODE_ERROR` coverage.
- The default skeleton codec path remains unchanged; the new status is only returned by the optional CRC32 wire decode path.

## Residual

- This does not implement MAC or timestamp semantics.
- End-to-end supervisor policy still treats non-OK codec results as decode failure; CRC-specific reporting beyond the codec boundary remains future integration policy growth.
