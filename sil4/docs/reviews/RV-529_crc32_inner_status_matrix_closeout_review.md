# RV-529 CRC32 Inner Status Matrix Closeout Review

## Document Control

- Review ID: `RV-529`
- Scope: `R-006`, `TC-CODEC-033`, `TC-CODEC-036`, `TC-CODEC-059..063`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The CRC32 wrapper inner-status preservation matrix is now representative-closeout for the current default codec decode status families that can occur after CRC validation succeeds.

The current matrix covers authenticated malformed or unsupported payload/header cases and authenticated transport metadata misuse:

- reserved header tamper: `TC-CODEC-033`
- non-frame event and invalid channel metadata: `TC-CODEC-036`
- unsupported reason: `TC-CODEC-059`
- trailing bytes: `TC-CODEC-060`
- truncated payload: `TC-CODEC-061`
- oversized declared payload: `TC-CODEC-062`
- unsupported message: `TC-CODEC-063`

## Evidence Reviewed

- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/docs/roadmap_status.md`
- `sil4/docs/reviews/RV-524_crc32_unsupported_reason_status_preservation_review.md`
- `sil4/docs/reviews/RV-525_crc32_trailing_bytes_status_preservation_review.md`
- `sil4/docs/reviews/RV-526_crc32_truncated_payload_status_preservation_review.md`
- `sil4/docs/reviews/RV-527_crc32_oversized_payload_status_preservation_review.md`
- `sil4/docs/reviews/RV-528_crc32_unsupported_message_status_preservation_review.md`

## Accepted Position

- CRC validation authenticates the wrapped bytes but does not convert malformed or unsupported decoded content into accepted content.
- The current wrapper matrix preserves direct decode typed status and stale-output clear behavior across the current representative decode failure families that remain reachable after CRC validation.
- `SHORT_HEADER` is intentionally not part of the valid-CRC inner-status matrix because `rsrx_codec_decode_frame_with_crc32()` rejects frames shorter than header plus CRC as `CRC_TRUNCATED` before an inner frame can be formed.

## Residual

- Add more wrapper inner-status tests only when a new direct decode status family is introduced or a new security profile changes the wrapper layering.
- This closeout does not add a non-none RaSTA SR checksum algorithm.
- This closeout does not add CRC-bearing redundancy PDU behavior.
- This closeout does not add MAC/security extension behavior.

## Verification Position

This is a document-only closeout review. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
