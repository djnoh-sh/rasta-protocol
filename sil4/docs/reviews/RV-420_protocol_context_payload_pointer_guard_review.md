# RV-420 Protocol Context Payload Pointer Guard Review

## Document Control

- Review ID: `RV-420`
- Scope: `TC-PC-030`, `R-001`, `SR-003`
- Status: `Accepted`
- Date: `2026-06-01`

## Review Summary

Protocol context outbound encode request construction now rejects inconsistent payload pointer/length input before the request reaches adapter or codec layers.

`rsrx_protocol_context_build_encode_request()` returns `RSRX_STATUS_INVALID_ARGUMENT` when `xPayloadLength > 0` and `puPayload == NULL`. The existing encode-failure clear contract also clears the caller-visible request before returning.

## Evidence Reviewed

- Code:
  - `sil4/src/rsrx_protocol_context.c`
- Tests:
  - `sil4/tests/unit/test_rsrx_protocol_context.c`
- Specifications:
  - `sil4/docs/verification/protocol_context_test_spec_draft.md`
  - `sil4/docs/design/lld/protocol_context_lld_draft.md`
  - `sil4/docs/traceability/traceability_matrix_initial.md`
  - `sil4/docs/roadmap_status.md`

## Accepted Behavior

- Null payload with zero length remains valid for payload-free outbound messages.
- Null payload with nonzero length is rejected at the protocol context API boundary.
- Rejected payload pointer misuse does not advance `uNextTxSequenceNumber`.
- Rejected payload pointer misuse clears stale encode request output.

## Residual

- Broader `R-001` residual remains richer confirm/retransmission ordering variants and future message-family parity.
- Adapter and codec payload guards remain in place as downstream defensive layers.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-420` is accepted only when that chain is green.
