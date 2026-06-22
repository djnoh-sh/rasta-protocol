# RV-263 Protocol Context Sequence Wrap Guard Review

## Scope

- prevent outbound sequence wraparound in `rsrx_protocol_context_build_encode_request`
- keep normal sequence progression unchanged below the wrap boundary
- add unit/spec/traceability/roadmap evidence for the guard

## Inputs Reviewed

- `sil4/src/rsrx_protocol_context.c`
- `sil4/tests/unit/test_rsrx_protocol_context.c`
- `sil4/docs/design/lld/protocol_context_lld_draft.md`
- `sil4/docs/verification/protocol_context_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `rsrx_protocol_context_build_encode_request` now rejects encode requests when the next tx sequence is already at `UINT32_MAX`, avoiding a transition to sequence `0`.
2. `TC-PC-020` verifies that `UINT32_MAX - 1` is still emitted as the last safe sequence and that the following boundary call returns `REJECTED` without mutating the counter.
3. This closes a concrete `R-001` sequencing hardening item without expanding numeric parity sampling.

## Decision

- Pass.
- Treat outbound sequence wraparound as guarded for the current protocol-context implementation baseline.
