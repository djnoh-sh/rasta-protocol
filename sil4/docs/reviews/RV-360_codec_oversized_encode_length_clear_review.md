# RV-360 Codec Oversized Encode Length Clear Review

- Review ID: `RV-360`
- Date: `2026-05-19`
- Scope: oversized encode failure stale length clear.

## Findings

- `TC-CODEC-012` now starts with a stale non-zero `xEncodedLength` before the direct oversized-payload encode failure.
- The test verifies `RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE` and confirms `xEncodedLength` is cleared to `0`.
- This closes the oversized-payload branch of the broader encode failure length-clear contract.

## Residual

- Direct oversized encode failure no longer leaves stale encoded length in the current test evidence.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
