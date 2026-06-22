# RV-343 CRC-Required Public API Init Review

- Review ID: `RV-343`
- Date: `2026-05-07`
- Scope: public API session initialization behavior for CRC-required deployments.

## Findings

- `rsrx_session_init()` already routes configs through `rsrx_validate_session_config()`.
- `TC-API-014` verifies that `uRequireCrc=1` rejects the default non-CRC codec port at the public API boundary.
- The same test verifies that selecting `rsrx_codec_get_crc32_port()` satisfies the policy and leaves the initialized session in the expected initial state.

## Residual

- This confirms policy enforcement at public API startup, but does not change the default codec policy.
- MAC, timestamp, PDU parity, and stronger security-field implementations remain future codec-security growth.
