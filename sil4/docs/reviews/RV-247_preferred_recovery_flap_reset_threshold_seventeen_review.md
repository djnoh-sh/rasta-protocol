# RV-247 Preferred Recovery Flap Reset Threshold Seventeen Review

## Scope

- extend preferred recovery flap-reset representative parity from `2..16` to `2..17`
- keep preferred recovery holdoff parity at `2..17`
- align roadmap/spec/traceability wording to the new redundancy closeout state

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-CHM-043` extends direct channel-manager flap-reset parity to threshold `17`.
2. `TC-INT-188` extends integration-side flap-reset parity to threshold `17`.
3. current preferred recovery threshold closeout now reads as pure parity `3..17` plus flap-reset parity `3..17`.
4. current `R-003` wording can now treat holdoff/flap-reset representative parity as closed through `2..17`, leaving broader policy growth outside the current family as the main residual.

## Decision

- pass
- treat preferred recovery flap-reset representative coverage as current through `2..17`

## Follow-Up

- evaluate threshold family growth beyond current representative parity `2..17`
- evaluate broader redundancy policy growth outside the current switch-audit envelope
