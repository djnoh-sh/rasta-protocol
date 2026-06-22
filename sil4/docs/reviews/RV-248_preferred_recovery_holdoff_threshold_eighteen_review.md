# RV-248 Preferred Recovery Holdoff Threshold Eighteen Review

## Scope

- extend preferred recovery holdoff parity from `2..17` to `2..18`
- keep flap-reset representative parity at `2..17`
- align roadmap/spec/traceability wording to the new redundancy state

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-CHM-044` extends direct channel-manager preferred recovery holdoff parity to threshold `18`.
2. `TC-INT-189` extends integration-side preferred recovery holdoff parity to threshold `18`.
3. current preferred recovery threshold closeout now reads as pure parity `3..18` plus flap-reset parity `3..17`.
4. current `R-003` wording now reflects holdoff parity `2..18`, flap-reset representative parity `2..17`, and direct terminal outcome coverage `3..17`.

## Decision

- pass
- treat preferred recovery holdoff parity as current through `2..18`
- keep flap-reset representative coverage scoped through `2..17`

## Follow-Up

- extend flap-reset parity beyond current `2..17`
- evaluate threshold family growth beyond current representative parity `2..18`
