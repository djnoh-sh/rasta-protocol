# RV-342 CRC-Required Startup Policy Gate Review

- Review ID: `RV-342`
- Date: `2026-05-07`
- Scope: configuration validation for deployments that require CRC protection.

## Findings

- `rsrx_session_config_t` now includes `uRequireCrc` as an explicit deployment policy flag.
- `rsrx_validate_session_config()` rejects `uRequireCrc=1` unless the selected codec port is the CRC32 encode/decode port.
- `TC-CFG-009` verifies default codec rejection and CRC32 codec acceptance for the same required-CRC policy.

## Residual

- This does not make CRC32 the default deployment policy.
- MAC, timestamp, PDU parity, and stronger security-field implementations remain future codec-security growth.
