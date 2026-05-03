# RV-306 Codec Null Payload Length Guard Review

## Scope

- Review ID: `RV-306`
- Scope: `codec encode null payload with non-zero length guard`
- Date: 2026-05-04

## Findings

1. `TC-CODEC-008` verifies that encode rejects a null payload pointer when payload length is non-zero.
2. The guard prevents payload-copy dereference on invalid caller input.
3. Zero-length payload with null pointer remains available for control messages because the new guard only applies when length is non-zero.

## Disposition

- Pass.
- Codec encode input validation now distinguishes empty payload from invalid payload pointer/length mismatch.
