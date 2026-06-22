# RV-369 Codec Decode Null Argument Clear Review

## Scope

- `R-006` codec/security stale output policy
- `TC-CODEC-016` direct decode null-argument output clear
- `TC-CODEC-031` CRC32 decode null-argument output clear

## Review Questions

1. Does a decode failure with a valid decoded-message output clear stale decoded state before returning?
2. Are direct and CRC32 decode paths aligned for null frame and null payload failures?
3. Do documentation and traceability describe the output-clear behavior without claiming a null output pointer can be cleared?

## Findings

1. Direct decode now validates the decoded-message output first, clears it, then rejects null frame or null payload arguments.
2. CRC32 decode applies the same ordering before CRC truncation or mismatch checks.
3. `TC-CODEC-016` and `TC-CODEC-031` seed stale decoded output and assert clear for the null frame and null payload paths.
4. LLD, test spec, traceability, and roadmap now capture this as codec stale-output policy hardening.

## Conclusion

- Pass. Decode null-argument failures with a valid output object no longer leave stale decoded state behind.

## Residual

- `R-006` remains focused on MAC/timestamp/PDU parity, additional tamper taxonomy, and vendor-oriented security negative vectors.
