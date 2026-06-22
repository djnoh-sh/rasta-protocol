# RV-373 Unsupported Reason Decode Clear Review

## Scope

- `R-006` codec/security stale output policy
- `TC-CODEC-020` unsupported reason decode output clear

## Review Questions

1. Does unsupported reason-code rejection avoid leaving stale decoded-message output?
2. Is the failure still reported as `RSRX_CODEC_STATUS_UNSUPPORTED_REASON`?
3. Are spec, traceability, and roadmap aligned with the strengthened evidence?

## Findings

1. `TC-CODEC-020` now seeds stale decoded output before invoking the unsupported reason decode path.
2. The test asserts both `RSRX_CODEC_STATUS_UNSUPPORTED_REASON` and decoded-output clear.
3. No production code change was required because direct decode already clears a valid output object before reason-code validation.
4. Test spec, traceability, roadmap, and this review now capture the stale-output behavior.

## Conclusion

- Pass. Unsupported reason rejection now preserves typed status while clearing stale decoded state.

## Residual

- `R-006` remains focused on MAC/timestamp/PDU parity, additional tamper taxonomy, and vendor-oriented security negative vectors.
