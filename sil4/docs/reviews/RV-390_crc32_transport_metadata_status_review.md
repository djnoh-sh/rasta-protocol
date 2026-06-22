# RV-390 CRC32 Transport Metadata Status Review

## Scope

- `R-006` codec/security negative-vector growth
- `TC-CODEC-036` CRC32 wrapper transport metadata misuse coverage
- Stale decoded-message output clear behavior

## Review Questions

1. Does CRC32 decode preserve direct codec typed status when checksum-valid frames carry non-frame transport events?
2. Does CRC32 decode preserve direct codec typed status when checksum-valid frames carry invalid channel metadata?
3. Does each rejected path clear stale decoded-message output?

## Findings

1. `TC-CODEC-036` first creates a valid CRC32 frame through the public CRC32 encoder.
2. The test then changes only transport metadata, not the authenticated payload bytes.
3. `SEND_COMPLETED` metadata returns `RSRX_CODEC_STATUS_NON_FRAME_EVENT`.
4. `RSRX_TRANSPORT_CHANNEL_INVALID` metadata returns `RSRX_CODEC_STATUS_INVALID_CHANNEL`.
5. Both paths verify stale decoded-message output is cleared.

## Conclusion

- Pass. CRC32 wrapper decode now has explicit evidence that checksum-valid payloads do not mask transport metadata misuse and that the direct codec's typed misuse status is preserved.

## Residual

- `R-006` still requires actual MAC/timestamp/PDU parity and vendor-oriented security negative vectors.
