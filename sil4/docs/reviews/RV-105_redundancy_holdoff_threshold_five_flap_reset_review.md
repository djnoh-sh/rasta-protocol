# RV-105 Redundancy Holdoff Threshold Five Flap-Reset Review

## Scope
- `R-003` holdoff threshold `5` flap-reset parity
- unit/integration parity for higher-threshold reset interaction beyond current `3/4`

## Inputs Reviewed
- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings
1. `TC-CHM-019` verifies that after one accumulated hold in a holdoff `5` configuration, a primary flap reset clears the stable-selection count and requires five fresh stable recovery selections before primary recovery is allowed again.
2. `TC-INT-130` lifts the same contract through the session/supervisor path and confirms the session remains `ESTABLISHED` after the renewed fifth recovery refresh and subsequent primary inbound success.
3. `TC-CHM-016` and `TC-INT-126` were widened again so threshold closeout coverage now tracks `3/4/5` plus all currently-covered flap-reset variants.

## Decision
- Accept.
- `R-003` higher-threshold flap-reset residual is narrower again; the current parity set is covered through threshold `5`, and the next gap is broader threshold generalization policy rather than another immediate reset-parity hole.
