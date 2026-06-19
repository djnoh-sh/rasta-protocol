# RV-531 RaSTA SR Disconnect Reason Delta Cleanup Review

## Document Control

- Review ID: `RV-531`
- Scope: `PDU-PARITY-001`, `TC-CODEC-039`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The RaSTA SR PDU wire-profile draft still listed disconnect-reason mapping tests as a required delta even though `TC-CODEC-039` already covers the mapping contract.

This review corrects the stale delta wording without changing implementation behavior.

## Evidence Reviewed

- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/include/rsrx_codec.h`
- `sil4/src/rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_codec.c`

## Accepted Position

- `rsrx_rasta_disconnect_reason_t` defines the current RaSTA disconnect reason numeric boundary.
- `rsrx_codec_map_reason_to_rasta_disconnect_reason()` maps supported internal disconnect reasons to repo-source RaSTA DiscReq reason values.
- `TC-CODEC-039` verifies supported mappings and unsupported/null-output clear behavior, including out-of-range reason handling.
- The `Required SIL4 Delta` table should therefore mark internal reason-byte parity as implemented for the current mapping boundary, not as an open test gap.

## Residual

- Official/customer specification clause IDs still need to be attached before claiming formal RaSTA conformance.
- Additional disconnect reason mappings should be added only if a new internal reason or controlled requirement introduces a new DiscReq reason boundary.

## Verification Position

This is a document-only consistency cleanup. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
