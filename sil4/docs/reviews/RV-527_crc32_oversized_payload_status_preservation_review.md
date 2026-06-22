# RV-527 CRC32 Oversized Payload Status Preservation Review

## Document Control

- Review ID: `RV-527`
- Scope: `TC-CODEC-062`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The CRC32 decode wrapper now has explicit evidence that a valid checksum does not mask an oversized declared-payload failure from the inner codec decode path.

`TC-CODEC-062` constructs a frame whose authenticated header declares `D_RSRX_CODEC_MAX_PAYLOAD_BYTES + 1` payload bytes before the CRC32 trailer. The wrapper must return `RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE` and clear stale decoded-message output.

## Evidence Reviewed

- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/docs/roadmap_status.md`

## Accepted Position

- CRC validation does not relax the bounded decoded payload capacity.
- An authenticated header declaring an oversized payload remains a typed decode rejection.
- The CRC32 wrapper preserves the direct decode stale-output clear contract on this negative path.

## Residual

- This review does not add a non-none RaSTA SR checksum algorithm.
- This review does not add CRC-bearing redundancy PDU behavior.
- This review does not add MAC/security extension behavior.

## Verification Position

The full local verification sequence passed for this code/test/spec update: clean configure, build, unit/integration executables, then cppcheck.
