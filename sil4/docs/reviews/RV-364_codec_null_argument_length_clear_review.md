# RV-364 Codec Null Argument Length Clear Review

- Review ID: `RV-364`
- Date: `2026-05-20`
- Scope: null-argument encode failure stale length clear

## Findings

- `TC-CODEC-016` starts clearable null-argument encode failures with a stale non-zero `xEncodedLength`.
- The test verifies null encode request and null encode output buffer failures return `RSRX_CODEC_STATUS_INVALID_ARGUMENT` and clear `xEncodedLength` to `0`.
- The null encode buffer case cannot clear `xEncodedLength` because no valid output buffer record is available.

## Residual

- Clearable null-argument encode failures no longer leave stale encoded length in the current test evidence.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
