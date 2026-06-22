# RV-526 CRC32 Truncated Payload Status Preservation Review

## Document Control

- Review ID: `RV-526`
- Scope: `TC-CODEC-061`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The CRC32 decode wrapper now has explicit evidence that a valid checksum does not mask a truncated-payload failure from the inner codec decode path.

`TC-CODEC-061` constructs a frame whose authenticated header declares one payload byte while the authenticated payload body is absent before the CRC32 trailer. The wrapper must return `RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD` and clear stale decoded-message output.

## Evidence Reviewed

- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/docs/roadmap_status.md`

## Accepted Position

- CRC validation does not relax the declared payload-length lower bound.
- An authenticated header with a missing payload body remains a typed decode rejection.
- The CRC32 wrapper preserves the direct decode stale-output clear contract on this negative path.

## Residual

- This review does not add a non-none RaSTA SR checksum algorithm.
- This review does not add CRC-bearing redundancy PDU behavior.
- This review does not add MAC/security extension behavior.

## Verification Position

The full local verification sequence passed for this code/test/spec update: clean configure, build, unit/integration executables, then cppcheck.
