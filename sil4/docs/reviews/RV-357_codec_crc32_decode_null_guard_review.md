# RV-357 Codec CRC32 Decode Null Guard Review

- Review ID: `RV-357`
- Date: `2026-05-19`
- Scope: CRC32 decode wrapper null argument guard.

## Findings

- `rsrx_codec_decode_frame_with_crc32()` now rejects a null decoded-message output pointer before CRC/truncation/mismatch processing.
- `TC-CODEC-031` verifies null frame, null decoded-message output, and null payload pointer paths all return `RSRX_CODEC_STATUS_INVALID_ARGUMENT`.
- This aligns the CRC32 wrapper with the direct default decode public API guard policy and prevents CRC-specific statuses from masking caller argument misuse.

## Residual

- CRC32 wrapper argument handling is now representative for null inputs.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
