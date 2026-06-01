# RV-412 RaSTA Redundancy Wire Profile Metadata Review

## Document Control

- Review ID: `RV-412`
- Scope: `TC-CODEC-047`, `RED-PDU-PARITY-001A`, `R-006`
- Status: `Accepted`
- Date: `2026-06-01`

## Review Summary

The SIL4 codec now exposes a metadata-only RaSTA redundancy PDU wire profile through `rsrx_codec_get_rasta_redundancy_wire_profile()`.

The profile fixes the repo-source redundancy PDU envelope as an 8-byte header carrying one maximum-size SR packet plus up to 4 bytes of redundancy CRC. This is an intentional contract boundary for future redundancy PDU encode/decode work; it does not change current channel-manager behavior and does not claim RaSTA CRC option a-e calculation parity.

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

- `D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES` is `8`.
- `D_RSRX_CODEC_RASTA_REDUNDANCY_MAX_CRC_BYTES` is `4`.
- `D_RSRX_CODEC_MAX_RASTA_REDUNDANCY_FRAME_BYTES` equals redundancy header plus max SR frame plus max redundancy CRC.
- The redundancy profile reports a distinct profile id, max carried SR frame payload capacity, max frame size, and CRC-present metadata.
- The profile is metadata-only and does not serialize or parse redundancy PDUs.

## Residual

- Redundancy PDU encode/decode behavior remains open if redundancy wire mode is in scope.
- RaSTA CRC option a-e implementations or explicit unsupported-option admission gates remain open.
- Target hardware CRC acceleration must remain behind the target codec/security adapter and requires software-vs-hardware equivalence evidence.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-412` is accepted only when that chain is green.
