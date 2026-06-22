# RV-533 RaSTA SR Wire Profile Current Baseline Alignment Review

## Document Control

- Review ID: `RV-533`
- Scope: `PDU-PARITY-001`, `TC-CODEC-037..041`, `TC-CODEC-055..057`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The RaSTA SR PDU wire-profile draft still described the current SIL4 codec baseline primarily as the 16-byte default skeleton profile, even though the selected no-checksum RaSTA SR host path is now implemented and tested.

This review aligns the document wording by separating the legacy/default skeleton profile from the selected RaSTA SR no-checksum host baseline.

## Evidence Reviewed

- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/design/rasta_normative_feature_inventory.md`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/include/rsrx_codec.h`
- `sil4/src/rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_codec.c`

## Accepted Position

- The default 16-byte codec profile remains useful for bounded internal codec and supervisor evidence.
- The selected RaSTA SR no-checksum host profile is separately implemented through a 28-byte fixed-header path with explicit field contracts, fixed big-endian byte order, supported numeric type mapping, timestamp fields, identity fields, and zero-checksum metadata.
- Remaining deltas are conditional: non-none checksum/hash calculation, CRC-bearing redundancy behavior, target timestamp-source evidence, official/customer clause mapping, and optional MAC/security extension.

## Decision

Accepted as a document-only baseline wording cleanup. The wire-profile draft should not imply that all SR common-field parity is still absent when the selected no-checksum host baseline is already implemented.

## Verification Position

This is a document-only consistency cleanup. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
