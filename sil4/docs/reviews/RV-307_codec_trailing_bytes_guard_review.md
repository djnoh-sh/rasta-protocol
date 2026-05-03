# RV-307 Codec Trailing Bytes Guard Review

## Scope

- Review ID: `RV-307`
- Scope: `codec decode trailing bytes negative vector`
- Date: 2026-05-04

## Findings

1. `TC-CODEC-009` verifies that a frame with declared zero payload length and extra trailing byte is rejected with `DECODE_ERROR`.
2. The implementation already requires exact equality between declared payload length and actual frame length; the new test fixes the trailing-data direction explicitly.
3. This strengthens vendor-evidence-friendly codec negative vectors without changing the wire format.

## Disposition

- Pass.
- Codec length mismatch handling is now represented by an explicit trailing-byte reject test.
