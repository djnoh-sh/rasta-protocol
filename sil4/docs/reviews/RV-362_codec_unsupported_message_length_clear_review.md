# RV-362 Codec Unsupported Message Length Clear Review

- Review ID: `RV-362`
- Date: `2026-05-19`
- Scope: unsupported-message encode failure stale length clear

## Findings

- `TC-CODEC-015` starts with a stale non-zero `xEncodedLength` before the unsupported outbound message type failure.
- The test verifies `RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE` and confirms `xEncodedLength` is cleared to `0`.
- This closes the unsupported-message branch of the broader encode failure length-clear contract.

## Residual

- Unsupported outbound message encode failure no longer leaves stale encoded length in the current test evidence.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
