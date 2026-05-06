# RV-333 Codec CRC32 Primitive Review

## Scope

- Review ID: `RV-333`
- Scope: deterministic CRC32 primitive for future codec integrity growth
- Date: 2026-05-06

## Findings

1. `rsrx_codec_calculate_crc32()` adds a public CRC32 primitive without changing the current wire format.
2. `TC-CODEC-025` verifies the standard `123456789` CRC32 vector `0xCBF43926`.
3. `TC-CODEC-025` also verifies null-output and null non-empty input rejection, while allowing empty input to produce CRC `0`.
4. The wire profile still reports CRC as absent, so this step does not claim PDU-level CRC parity.

## Disposition

- Pass.
- CRC implementation risk is reduced to a deterministic primitive before the later wire-format integration decision.
