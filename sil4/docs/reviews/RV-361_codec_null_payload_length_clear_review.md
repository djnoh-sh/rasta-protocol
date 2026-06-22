# RV-361 Codec Null Payload Length Clear Review

- Review ID: `RV-361`
- Date: `2026-05-19`
- Scope: null-payload encode failure stale length clear

## Findings

- `TC-CODEC-008` starts with a stale non-zero `xEncodedLength` before the null-payload-with-nonzero-length encode failure.
- The test verifies `RSRX_CODEC_STATUS_INVALID_ARGUMENT` and confirms `xEncodedLength` is cleared to `0`.
- This closes the caller-misuse null-payload branch of the broader encode failure length-clear contract.

## Residual

- Direct null-payload-with-length encode failure no longer leaves stale encoded length in the current test evidence.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
