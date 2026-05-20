# RV-366 Codec Short Header Decode Clear Review

- Review ID: `RV-366`
- Date: `2026-05-20`
- Scope: short-header decode failure stale output clear

## Findings

- `rsrx_codec_decode_frame()` now clears a valid decoded-message output record before applying decode validation.
- `rsrx_codec_decode_frame_with_crc32()` applies the same clear behavior before CRC-specific failure checks when a decoded-message output record is valid.
- `TC-CODEC-017` now seeds stale decoded output before a short-header failure and verifies the output record is reset to safe defaults.

## Residual

- Short-header decode failure no longer leaves stale decoded output in the current test evidence.
- Additional decode failure statuses can reuse the same clear contract, but broader MAC/timestamp/PDU parity remains future codec-security implementation growth.
