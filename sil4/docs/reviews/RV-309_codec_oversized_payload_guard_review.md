# RV-309 Codec Oversized Payload Guard Review

## Scope

- Review ID: `RV-309`
- Scope: `codec decode oversized declared payload negative vector`
- Date: 2026-05-04

## Findings

1. `TC-CODEC-011` verifies that a frame declaring `D_RSRX_CODEC_MAX_PAYLOAD_BYTES + 1` bytes is rejected with `PAYLOAD_TOO_LARGE`.
2. The test uses a frame whose actual length matches the declared oversized length, so the evidence targets the max-payload guard rather than the length-mismatch guard.
3. This closes the explicit bounded decoded-payload evidence gap without changing the current wire format.

## Disposition

- Pass.
- Codec decode now has direct unit/spec/review evidence for rejecting payload lengths that exceed the bounded decoded payload buffer.
