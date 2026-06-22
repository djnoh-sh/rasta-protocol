# RV-411 RaSTA SR Selected Checksum Profile Review

## Document Control

- Review ID: `RV-411`
- Scope: `TC-CODEC-046`, `PDU-PARITY-001E`, `R-006`
- Status: `Accepted`
- Date: `2026-06-01`

## Review Summary

The SIL4 codec now exposes the selected default RaSTA SR checksum profile through `rsrx_codec_get_rasta_sr_default_checksum_profile()`. The selected baseline is explicitly `RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE / 0 bytes`.

This makes the current no-checksum SR profile an intentional implementation boundary rather than an implicit absence of checksum logic. It also keeps MD4, BLAKE2b, and SipHash-2-4 behind the existing unsupported-profile admission rejection until a controlled requirement selects a non-none profile.

## Evidence Reviewed

- Code:
  - `sil4/include/rsrx_codec.h`
  - `sil4/src/rsrx_codec.c`
- Tests:
  - `sil4/tests/unit/test_rsrx_codec.c`
- Specifications:
  - `sil4/docs/verification/protocol_codec_test_spec_draft.md`
  - `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
  - `sil4/docs/design/rasta_normative_feature_inventory.md`
  - `sil4/docs/roadmap_status.md`

## Accepted Behavior

- The selected default checksum profile pointer is non-null.
- The selected default algorithm is `RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE`.
- The selected default checksum length is `0`.
- The selected default profile passes `rsrx_codec_validate_rasta_sr_checksum_profile()`.
- MD4, BLAKE2b, and SipHash-2-4 remain unsupported unless selected by a controlled requirement and implemented with vectors.

## Residual

- This review does not claim MD4, BLAKE2b, or SipHash calculation.
- If a non-none checksum profile is selected later, implementation must include algorithm code, golden vectors, malformed checksum negative vectors, profile configuration evidence, and software-vs-hardware equivalence evidence when target acceleration is used.
- Redundancy PDU CRC options remain separate from SR checksum/hash behavior.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-411` is accepted only when that chain is green.
