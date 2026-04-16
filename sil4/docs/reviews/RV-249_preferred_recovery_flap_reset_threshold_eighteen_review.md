# RV-249 Preferred Recovery Flap Reset Threshold Eighteen Review

## Scope

- extend preferred recovery flap-reset representative parity from `2..17` to `2..18`
- keep preferred recovery pure holdoff parity at `2..18`
- align roadmap/spec/traceability wording to the new redundancy state

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-CHM-045` extends direct channel-manager preferred recovery flap-reset parity to threshold `18`.
2. `TC-INT-190` extends integration-side preferred recovery flap-reset parity to threshold `18`.
3. current preferred recovery threshold closeout now reads as pure parity `3..18` plus flap-reset parity `3..18`.
4. current `R-003` wording now reflects preferred recovery holdoff/flap-reset parity `2..18` and direct terminal outcome coverage `3..17`.

## Decision

- pass
- treat preferred recovery holdoff/flap-reset parity as current through `2..18`

## Follow-Up

- evaluate threshold family growth beyond current representative parity `2..18`
- extend redundancy policy beyond the current switch-audit envelope
