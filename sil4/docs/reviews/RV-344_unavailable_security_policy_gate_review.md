# RV-344 Unavailable Security Policy Gate Review

- Review ID: `RV-344`
- Date: `2026-05-07`
- Scope: configuration validation for MAC/timestamp policies that are not implemented by the current codec family.

## Findings

- `rsrx_session_config_t` now exposes `uRequireMac` and `uRequireTimestamp` as explicit deployment policy flags.
- `rsrx_validate_session_config()` rejects MAC-required and timestamp-required configurations with `INCONSISTENT_VALUE` on `CODEC_PORT`.
- `TC-CFG-010` verifies that unavailable MAC/timestamp policies fail at startup instead of silently falling back to the default skeleton or CRC32-only codec path.

## Residual

- This is a deterministic unsupported-policy gate, not a MAC or timestamp implementation.
- Actual MAC, timestamp, and PDU parity remain future codec-security implementation work.
