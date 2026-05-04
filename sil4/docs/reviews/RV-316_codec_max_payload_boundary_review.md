# RV-316 Codec Max Payload Boundary Review

## Scope

- Review ID: `RV-316`
- Scope: `codec max payload encode/decode boundary`
- Date: 2026-05-04

## Findings

1. `TC-CODEC-018` verifies that a payload exactly equal to `D_RSRX_CODEC_MAX_PAYLOAD_BYTES` is encoded successfully.
2. The same test decodes the resulting `D_RSRX_CODEC_MAX_FRAME_BYTES` frame and verifies payload length and boundary bytes are preserved.
3. This complements the oversized-payload reject tests by proving the configured maximum is inclusive, not off by one.

## Disposition

- Pass.
- Codec maximum payload boundary now has direct unit/spec/review evidence for the accepted edge.
