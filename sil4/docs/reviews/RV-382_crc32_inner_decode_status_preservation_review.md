# RV-382 CRC32 Inner Decode Status Preservation Review

## Scope

- `R-006` codec/security typed status taxonomy
- `TC-CODEC-033` CRC32 wrapper payload decode status preservation

## Review Questions

1. Does CRC32 decode distinguish checksum mismatch from a malformed payload that has a valid checksum?
2. Does the CRC32 wrapper preserve the inner direct-decode typed status after checksum validation succeeds?
3. Does the stale decoded-message output remain cleared on the preserved inner failure?

## Findings

1. `TC-CODEC-033` builds a frame with non-zero reserved header bytes and a matching CRC32 checksum over that malformed payload.
2. `rsrx_codec_decode_frame_with_crc32` validates the checksum first, then delegates to direct payload decode.
3. The test verifies that the resulting status is `RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO`, not `RSRX_CODEC_STATUS_CRC_MISMATCH`.
4. The test also verifies that stale decoded-message output is cleared on the preserved inner failure.
5. No production code change was required because the wrapper already delegates after successful checksum validation.

## Conclusion

- Pass. The CRC32 wrapper now has direct evidence that authenticated malformed payloads retain the default codec's typed decode taxonomy.

## Residual

- `R-006` remains focused on actual MAC/timestamp/PDU parity, additional tamper taxonomy, and vendor-oriented security negative vectors.
