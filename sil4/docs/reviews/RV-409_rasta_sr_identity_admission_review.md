# RV-409 RaSTA SR Identity Admission Review

## Document Control

- Review ID: `RV-409`
- Scope: `TC-CODEC-045`, `PDU-PARITY-001F`, `R-006`
- Status: `Accepted`
- Date: `2026-05-29`

## Review Summary

The codec now exposes RaSTA SR receiver/sender identity admission. A decoded SR packet can be checked against an expected local receiver ID and expected remote sender ID before it is mapped to the internal decoded-message handoff contract.

This creates a typed authenticity boundary for endpoint ID mismatch without claiming MAC or cryptographic authentication.

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

- Matching receiver/sender IDs return `RSRX_CODEC_STATUS_OK`.
- Receiver ID mismatch returns `RSRX_CODEC_STATUS_RECEIVER_ID_MISMATCH`.
- Sender ID mismatch returns `RSRX_CODEC_STATUS_SENDER_ID_MISMATCH`.
- Null packet/policy and zero expected endpoint IDs return `RSRX_CODEC_STATUS_INVALID_ARGUMENT`.
- Identity+timestamp handoff rejects mismatched endpoint IDs before message mapping and clears stale decoded-message output.

## Residual

- Supervisor/runtime selection still needs deployment identity policy wiring.
- This is endpoint ID admission, not cryptographic authentication.
- MAC/security extension and selected checksum/hash calculation remain controlled-requirement follow-up items.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-409` is accepted only when that chain is green.
