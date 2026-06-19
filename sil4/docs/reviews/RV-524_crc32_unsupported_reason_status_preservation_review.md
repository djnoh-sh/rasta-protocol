# RV-524 CRC32 Unsupported Reason Status Preservation Review

## Document Control

- Review ID: `RV-524`
- Scope: `TC-CODEC-059`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The CRC32 decode wrapper now has explicit evidence that a valid checksum does not mask an unsupported reason-code failure from the inner codec decode path.

`TC-CODEC-059` constructs a minimal frame with a valid CRC32 trailer and an unsupported reason byte. The wrapper must return `RSRX_CODEC_STATUS_UNSUPPORTED_REASON` and clear stale decoded-message output.

## Evidence Reviewed

- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/docs/roadmap_status.md`

## Accepted Position

- CRC validation is an authenticity/integrity gate for the wrapped bytes, not an admission decision for semantically unsupported header fields.
- The CRC32 wrapper preserves the direct decode typed-status contract after checksum validation succeeds.
- Stale decoded-message output is cleared on the authenticated unsupported-reason negative path.

## Residual

- This review does not add a non-none RaSTA SR checksum algorithm.
- This review does not add CRC-bearing redundancy PDU behavior.
- This review does not add MAC/security extension behavior.

## Verification Position

The full local verification sequence passed for this code/test/spec update: clean configure, build, unit/integration executables, then cppcheck.
