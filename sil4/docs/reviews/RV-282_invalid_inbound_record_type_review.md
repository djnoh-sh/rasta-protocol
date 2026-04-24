- Scope: `R-001 invalid inbound record type reject`

## Review Question

1. Does `rsrx_protocol_context_record_inbound_message` reject inbound message types outside the protocol context inbound contract?
2. Does that rejection preserve sequence, confirmation, and retransmission tracking state so invalid record misuse cannot contaminate protocol context?
3. Do LLD, verification spec, traceability, and roadmap describe this as current sequencing hardening rather than broader protocol growth?

## Evidence Reviewed

- `sil4/src/rsrx_protocol_context.c`
- `sil4/tests/unit/test_rsrx_protocol_context.c`
- `sil4/docs/design/lld/protocol_context_lld_draft.md`
- `sil4/docs/verification/protocol_context_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `record_inbound_message` now rejects `MESSAGE_TYPE_INVALID` with `INVALID_ARGUMENT` before any sequence, confirmation, or retransmission state update.
2. The new unit test confirms invalid inbound record misuse preserves next-tx, last-rx, tx-confirmation, remote-confirmation, retransmission-pending, retransmission-base, and latest retransmission-request-tx state.
3. Documentation and roadmap updates keep this item framed as protocol-context sequencing hardening inside current `R-001` closeout scope.

## Conclusion

- Reviewed change is acceptable.
- Residual `R-001` scope remains future confirm/retransmission ordering variants and broader integration parity, not inbound record type misuse guarding.
