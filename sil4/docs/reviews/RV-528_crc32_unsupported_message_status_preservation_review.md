# RV-528 CRC32 Unsupported Message Status Preservation Review

## Document Control

- Review ID: `RV-528`
- Scope: `TC-CODEC-063`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The CRC32 decode wrapper now has explicit evidence that a valid checksum does not mask an unsupported message-type failure from the inner codec decode path.

`TC-CODEC-063` constructs a frame whose authenticated header contains `RSRX_MESSAGE_TYPE_INVALID`. The wrapper must return `RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE` and clear stale decoded-message output.

## Evidence Reviewed

- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/docs/roadmap_status.md`

## Accepted Position

- CRC validation authenticates the wrapped bytes but does not admit unsupported message types.
- An authenticated unsupported message type remains a typed decode rejection.
- The CRC32 wrapper preserves the direct decode stale-output clear contract on this negative path.

## Residual

- This review does not add a non-none RaSTA SR checksum algorithm.
- This review does not add CRC-bearing redundancy PDU behavior.
- This review does not add MAC/security extension behavior.

## Verification Position

The full local verification sequence passed for this code/test/spec update: clean configure, build, unit/integration executables, then cppcheck.
