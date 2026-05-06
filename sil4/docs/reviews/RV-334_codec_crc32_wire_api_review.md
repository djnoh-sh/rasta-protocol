# RV-334 Codec CRC32 Wire API Review

## Scope

- Review ID: `RV-334`
- Scope: optional CRC32 wire encode/decode API
- Date: 2026-05-06

## Findings

1. `rsrx_codec_encode_message_with_crc32()` appends a CRC32 trailer to the existing skeleton frame without changing the default `rsrx_codec_encode_message()` behavior.
2. `rsrx_codec_decode_frame_with_crc32()` verifies the CRC32 trailer before delegating to the existing skeleton decoder.
3. `TC-CODEC-026` verifies CRC32 wire round-trip, payload tamper rejection, and small-buffer rejection.
4. The default codec port remains bound to the non-CRC skeleton path, so integration behavior is unchanged until a higher-level policy selects the CRC32 path.

## Disposition

- Pass.
- CRC32 wire-format capability now exists as an explicit optional API; default integration policy remains a separate decision.
