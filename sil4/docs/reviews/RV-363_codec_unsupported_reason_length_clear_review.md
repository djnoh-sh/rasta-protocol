# RV-363 Codec Unsupported Reason Length Clear Review

- Review ID: `RV-363`
- Date: `2026-05-20`
- Scope: unsupported-reason encode failure stale length clear

## Findings

- `TC-CODEC-020` starts with a stale non-zero `xEncodedLength` before the unsupported outbound reason code failure.
- The test verifies `RSRX_CODEC_STATUS_UNSUPPORTED_REASON` and confirms `xEncodedLength` is cleared to `0`.
- This closes the unsupported-reason branch of the broader encode failure length-clear contract.

## Residual

- Unsupported outbound reason encode failure no longer leaves stale encoded length in the current test evidence.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
