- Scope: `R-001 invalid outbound message type reject`

## Review Question

1. Does `rsrx_protocol_context_build_encode_request` reject outbound message types outside the protocol context contract?
2. Does that rejection preserve sequence state so invalid type misuse cannot perturb outbound ordering?
3. Do LLD, verification spec, traceability, and roadmap describe this as current sequencing hardening rather than broader new feature work?

## Evidence Reviewed

- `sil4/src/rsrx_protocol_context.c`
- `sil4/tests/unit/test_rsrx_protocol_context.c`
- `sil4/docs/design/lld/protocol_context_lld_draft.md`
- `sil4/docs/verification/protocol_context_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `build_encode_request` now rejects unsupported outbound message types with `INVALID_ARGUMENT` before sequence allocation.
2. The new unit test confirms `MESSAGE_TYPE_INVALID` leaves `uNextTxSequenceNumber` unchanged at its initial value.
3. Documentation and roadmap updates keep this item framed as protocol-context sequencing hardening inside current `R-001` closeout scope.

## Conclusion

- Reviewed change is acceptable.
- Residual `R-001` scope remains future confirm/retransmission ordering variants and broader integration parity, not outbound type misuse guarding.
