# RV-535 Protocol Codec Spec RaSTA Baseline Wording Review

## Document Control

- Review ID: `RV-535`
- Scope: `TC-CODEC-037..050`, `PDU-PARITY-001`, `RED-PDU-PARITY-001A`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The protocol codec verification spec still used staged wording such as "before SR encode/decode implementation" for some RaSTA SR and redundancy metadata tests. The selected no-checksum SR and option A redundancy paths are now implemented, so those notes should describe the current baseline rather than a future precondition.

## Evidence Reviewed

- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/docs/design/lld/protocol_codec_lld_draft.md`
- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/design/rasta_normative_feature_inventory.md`
- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_transport_supervisor.c`

## Accepted Position

- `TC-CODEC-037..046` support the selected no-checksum RaSTA SR host baseline, not just pre-implementation metadata.
- `TC-CODEC-047..050` support the option A no-CRC redundancy PDU baseline and carried SR decode bridge.
- Unsupported checksum profiles and CRC-bearing redundancy options remain safe rejection boundaries until selected by controlled requirement.

## Decision

Accepted as a document-only verification-spec wording cleanup. The tests and executable behavior are unchanged.

## Verification Position

This is a document-only verification specification cleanup. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
