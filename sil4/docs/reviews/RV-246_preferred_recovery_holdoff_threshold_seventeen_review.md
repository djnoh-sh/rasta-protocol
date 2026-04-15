# RV-246 Preferred Recovery Holdoff Threshold Seventeen Review

## Scope

- extend preferred recovery holdoff parity from `2..16` to `2..17`
- keep flap-reset representative parity at `2..16`
- align roadmap/spec/traceability wording to the new redundancy state

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`
- `sil4/docs/reviews/RV-244_switch_audit_threshold_seventeen_terminal_outcome_review.md`

## Findings

1. `TC-CHM-042` extends direct channel-manager preferred recovery holdoff parity to threshold `17`.
2. `TC-INT-187` extends integration-side preferred recovery holdoff parity to threshold `17`.
3. `TC-SUP-060` and `TC-INT-186` remove the duplicate ID collision introduced by the prior threshold-seventeen switch-audit closeout.
4. current `R-003` wording now reflects preferred recovery holdoff parity `2..17`, flap-reset representative parity `2..16`, and direct terminal outcome coverage `3..17`.

## Decision

- pass
- treat preferred recovery holdoff parity as current through `2..17`
- keep flap-reset representative coverage scoped through `2..16`

## Follow-Up

- extend flap-reset parity beyond current `2..16`
- evaluate broader redundancy policy growth outside the current switch-audit envelope
