# RV-405 RaSTA SR Checksum Profile Admission Review

## Document Control

- Review ID: `RV-405`
- Scope: `TC-CODEC-042`, `PDU-PARITY-001E`, `R-006`
- Status: `Accepted`
- Date: `2026-05-28`

## Review Summary

The SIL4 codec now exposes a RaSTA SR checksum profile admission contract. The current supported profile is explicitly `none / 0 bytes`. Configured MD4, BLAKE2b, and SipHash-2-4 checksum profiles with 8-byte or 16-byte checksum lengths are recognized but rejected with `RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE`.

This prevents accidental claims that the existing CRC32 wrapper is equivalent to RaSTA SR safety-code/hash behavior.

## Evidence Reviewed

- Code:
  - `sil4/include/rsrx_codec.h`
  - `sil4/src/rsrx_codec.c`
- Tests:
  - `sil4/tests/unit/test_rsrx_codec.c`
  - `sil4/tests/unit/test_rsrx_codec_contract.c`
- Specifications:
  - `sil4/docs/verification/protocol_codec_test_spec_draft.md`
  - `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
  - `sil4/docs/design/rasta_normative_feature_inventory.md`
  - `sil4/docs/roadmap_status.md`

## Accepted Behavior

- `none / 0 bytes` returns `RSRX_CODEC_STATUS_OK`.
- MD4, BLAKE2b, and SipHash-2-4 profiles with 8-byte or 16-byte checksum lengths return `RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE`.
- Inconsistent profiles, invalid checksum lengths, and null profile pointers return `RSRX_CODEC_STATUS_INVALID_ARGUMENT`.

## Residual

- Actual MD4, BLAKE2b, or SipHash calculation is not implemented and must not be claimed.
- If a controlled requirement selects a checksum profile, add algorithm implementation, golden vectors, malformed checksum negative vectors, and target software-vs-hardware equivalence evidence if acceleration is used.
- Timestamp/window admission and receiver/sender authenticity checks remain open.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-405` is accepted only when that chain is green.
