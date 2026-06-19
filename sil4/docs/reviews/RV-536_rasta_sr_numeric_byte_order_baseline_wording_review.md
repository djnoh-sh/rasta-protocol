# RV-536 RaSTA SR Numeric Type and Byte-Order Baseline Wording Review

## Document Control

- Review ID: `RV-536`
- Scope: `PDU-PARITY-001`, `TC-CODEC-039`, `TC-CODEC-040`, `TC-CODEC-056`, `TC-CODEC-057`, `TC-CODEC-058`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The RaSTA SR wire-profile draft still described numeric type handling and byte-order policy as future implementation conditions. The selected no-checksum SR path now encodes/decodes the supported current numeric type boundary with fixed big-endian byte order.

This review updates the wording so the document reflects the current baseline.

## Evidence Reviewed

- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/include/rsrx_codec.h`
- `sil4/src/rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_codec.c`

## Accepted Position

- Supported current RaSTA SR numeric message types are mapped and encoded/decoded through the selected no-checksum SR path.
- Unsupported `RetrResp` and `RetrData` remain explicit rejected mappings until their behavior is selected.
- Fixed big-endian byte order is the active selected SR profile policy, not only a future implementation decision.
- Legacy peer interoperability for host-endian repo-source behavior still requires deployment-specific golden-vector evidence before enabling any compatibility path.

## Verification Position

This is a document-only wording cleanup. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
