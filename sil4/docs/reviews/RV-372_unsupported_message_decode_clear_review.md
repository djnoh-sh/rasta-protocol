# RV-372 Unsupported Message Decode Clear Review

## Scope

- `R-006` codec/security stale output policy
- `TC-CODEC-005` unsupported message decode output clear

## Review Questions

1. Does unsupported message-type rejection avoid leaving stale decoded-message output?
2. Is the failure still reported as `RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE`?
3. Are spec, traceability, and roadmap aligned with the strengthened evidence?

## Findings

1. `TC-CODEC-005` now seeds stale decoded output before invoking the unsupported message decode path.
2. The test asserts both `RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE` and decoded-output clear.
3. No production code change was required because direct decode already clears a valid output object before message-type validation.
4. Test spec, traceability, roadmap, and this review now capture the stale-output behavior.

## Conclusion

- Pass. Unsupported message rejection now preserves typed status while clearing stale decoded state.

## Residual

- `R-006` remains focused on MAC/timestamp/PDU parity, additional tamper taxonomy, and vendor-oriented security negative vectors.
