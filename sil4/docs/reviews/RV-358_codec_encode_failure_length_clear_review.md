# RV-358 Codec Encode Failure Length Clear Review

- Review ID: `RV-358`
- Date: `2026-05-19`
- Scope: encode failure `xEncodedLength` clear contract.

## Findings

- `rsrx_codec_encode_message()` now clears `xEncodedLength` to `0` before validating a non-null encode buffer and only sets a non-zero length on success.
- `rsrx_codec_encode_message_with_crc32()` also clears the output length before delegation and after delegated encode failure.
- `TC-CODEC-032` verifies stale `xEncodedLength` is cleared for both direct and CRC32 encode failure paths.
- This prevents a failed encode result from carrying a previous successful length into adapter or caller logic.

## Residual

- Current direct and CRC32 encode failure paths have representative stale-length protection.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth.
