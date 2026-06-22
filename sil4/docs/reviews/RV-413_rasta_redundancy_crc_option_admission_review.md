# RV-413 RaSTA Redundancy CRC Option Admission Review

## Document Control

- Review ID: `RV-413`
- Scope: `TC-CODEC-048`, `CRC-PARITY-001`, `R-006`
- Status: `Accepted`
- Date: `2026-06-01`

## Review Summary

The SIL4 codec now exposes a RaSTA redundancy CRC option admission contract through `rsrx_codec_validate_rasta_redundancy_crc_profile()`.

The current supported redundancy CRC profile is option A with `0` CRC bytes. Options B and C are recognized as 4-byte CRC profiles, and options D and E are recognized as 2-byte CRC profiles, but they are rejected with `RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE` until actual RaSTA CRC option implementations and vectors are added.

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

- Option A with `0` CRC bytes returns `RSRX_CODEC_STATUS_OK`.
- Options B and C with `4` CRC bytes return `RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE`.
- Options D and E with `2` CRC bytes return `RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE`.
- Inconsistent option/width pairs and null profile pointers return `RSRX_CODEC_STATUS_INVALID_ARGUMENT`.

## Residual

- Actual RaSTA redundancy CRC option B/C/D/E calculation is not implemented and must not be claimed.
- If a non-A redundancy CRC option is selected later, implementation must include algorithm code, golden vectors, malformed CRC negative vectors, profile configuration evidence, and software-vs-hardware equivalence evidence if target acceleration is used.
- Redundancy PDU encode/decode behavior remains separate from CRC option admission.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-413` is accepted only when that chain is green.
