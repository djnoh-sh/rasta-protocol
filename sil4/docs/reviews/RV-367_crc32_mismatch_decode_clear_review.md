# RV-367 CRC32 Mismatch Decode Clear Review

- Review ID: `RV-367`
- Date: `2026-05-20`
- Scope: CRC32 mismatch decode failure stale output clear

## Findings

- `TC-CODEC-028` now seeds stale decoded output before a CRC32 checksum mismatch failure.
- The test verifies `RSRX_CODEC_STATUS_CRC_MISMATCH` and confirms the decoded-message output record is reset to safe defaults.
- This extends the decode-output clear contract to a security-relevant CRC/tamper failure path.

## Residual

- CRC32 mismatch decode failure no longer leaves stale decoded output in the current test evidence.
- CRC32 truncated checksum, additional direct decode status branches, and MAC/timestamp/PDU parity remain future codec-security growth candidates.
