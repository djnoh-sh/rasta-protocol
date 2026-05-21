# RV-370 CRC32 Truncated Decode Clear Review

## Scope

- `R-006` codec/security stale output policy
- `TC-CODEC-029` CRC32 truncated checksum output clear

## Review Questions

1. Does the CRC32 truncated checksum path avoid leaving stale decoded-message output?
2. Is the negative vector still reported as `RSRX_CODEC_STATUS_CRC_TRUNCATED`?
3. Are spec, traceability, and roadmap aligned with the strengthened evidence?

## Findings

1. `TC-CODEC-029` now seeds stale decoded output before invoking the truncated CRC32 decode path.
2. The test asserts both `RSRX_CODEC_STATUS_CRC_TRUNCATED` and decoded-output clear.
3. No production code change was required because CRC32 decode already clears a valid output object before truncation checks.
4. Test spec, traceability, roadmap, and this review now capture the stale-output behavior.

## Conclusion

- Pass. CRC32 checksum truncation is now a vendor-evidence-friendly negative vector that preserves typed status while clearing stale decoded state.

## Residual

- `R-006` remains focused on MAC/timestamp/PDU parity, additional tamper taxonomy, and vendor-oriented security negative vectors.
