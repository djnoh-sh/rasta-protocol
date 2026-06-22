# RV-407 RaSTA SR Timestamp Handoff Mapping Review

## Document Control

- Review ID: `RV-407`
- Scope: `TC-CODEC-044`, `PDU-PARITY-001F`, `R-006`
- Status: `Accepted`
- Date: `2026-05-29`

## Review Summary

The codec now provides a bridge from a decoded RaSTA SR packet to the existing internal `rsrx_decoded_message_t` handoff contract. The bridge first applies timestamp admission. Only admitted packets are mapped to message type, suggested event, reason, sequence, confirmation, and payload.

This keeps SR-specific timestamp policy enforceable before a decoded packet can reach protocol/session event resolution.

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

- Timestamp-admitted SR packets map to internal decoded messages.
- SR sequence and confirmed sequence map to internal sequence and confirmation fields.
- SR payload bytes are copied into the bounded decoded-message payload buffer.
- Timestamp admission failure returns the timestamp-specific codec status and clears stale decoded-message output.
- Unsupported SR message types and null inputs return typed failure and clear stale decoded-message output when possible.

## Residual

- This is a handoff bridge, not the final supervisor runtime SR codec selection path.
- Supervisor/runtime wiring must decide how a deployment selects the SR decode path and timestamp policy source.
- Receiver/sender ID authenticity checks remain open.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-407` is accepted only when that chain is green.
