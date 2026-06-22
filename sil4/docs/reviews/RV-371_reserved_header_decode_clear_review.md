# RV-371 Reserved Header Decode Clear Review

## Scope

- `R-006` codec/security stale output policy
- `TC-CODEC-007` reserved-header tamper output clear

## Review Questions

1. Does reserved-header tamper rejection avoid leaving stale decoded-message output?
2. Is the tamper-specific status still `RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO`?
3. Are spec, traceability, and roadmap aligned with the strengthened evidence?

## Findings

1. `TC-CODEC-007` now seeds stale decoded output before invoking the reserved-header tamper path.
2. The test asserts both `RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO` and decoded-output clear.
3. No production code change was required because direct decode already clears a valid output object before header validation.
4. Test spec, traceability, roadmap, and this review now capture the stale-output behavior.

## Conclusion

- Pass. Reserved-header tamper rejection is now a stronger security negative vector that preserves typed status while clearing stale decoded state.

## Residual

- `R-006` remains focused on MAC/timestamp/PDU parity, additional tamper taxonomy, and vendor-oriented security negative vectors.
