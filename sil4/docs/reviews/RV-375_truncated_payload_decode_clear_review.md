# RV-375 Truncated Payload Decode Clear Review

## Scope

- `R-006` codec/security stale output policy
- `TC-CODEC-010` truncated-payload decode output clear

## Review Questions

1. Does truncated-payload rejection avoid leaving stale decoded-message output?
2. Is the failure still reported as `RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD`?
3. Are spec, traceability, and roadmap aligned with the strengthened evidence?

## Findings

1. `TC-CODEC-010` now seeds stale decoded output before invoking the truncated-payload decode path.
2. The test asserts both `RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD` and decoded-output clear.
3. No production code change was required because direct decode already clears a valid output object before payload-length validation.
4. Test spec, traceability, roadmap, and this review now capture the stale-output behavior.

## Conclusion

- Pass. Truncated-payload rejection now preserves typed status while clearing stale decoded state.

## Residual

- `R-006` remains focused on MAC/timestamp/PDU parity, additional tamper taxonomy, and vendor-oriented security negative vectors.
