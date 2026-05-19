# RV-359 Codec Small Buffer Length Clear Review

- Review ID: `RV-359`
- Date: `2026-05-19`
- Scope: small-buffer encode failure stale length clear.

## Findings

- `TC-CODEC-006` now starts with a stale non-zero `xEncodedLength` before the direct small-buffer encode failure.
- The test verifies `RSRX_CODEC_STATUS_BUFFER_TOO_SMALL` and confirms `xEncodedLength` is cleared to `0`.
- This closes the representative small-buffer branch of the broader encode failure length-clear contract.

## Residual

- Direct small-buffer encode failure no longer leaves stale encoded length in the current test evidence.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
