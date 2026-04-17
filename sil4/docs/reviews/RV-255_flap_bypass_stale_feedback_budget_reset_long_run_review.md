# RV-255 Flap-Bypass Stale Feedback Budget-Reset Long-Run Review

## Scope

- add long-run integration coverage that combines flap-bypass stale feedback filtering with correlated send-budget reset in one representative path
- align feedback representative wrapper, flap-bypass closeout wrapper, roadmap, spec, and traceability wording to that new long-run state

## Inputs Reviewed

- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-INT-195` shows that inactive-channel stale `SEND_FAILED` events remain ignored across repeated flap-bypass cycles and do not contaminate the active send-failure budget.
2. After bypass refresh, only the active primary `SEND_FAILED` contributes to the local send-failure budget, and same-cycle primary success resets that budget before the next cycle rebuilds the same pattern.
3. `TC-INT-172` and `TC-INT-111` now describe the flap-bypass long-run family as including a dedicated stale-feedback budget-reset branch, not just filtering-only, stale-completion reset, and mixed-feedback reset branches.

## Decision

- pass
- treat flap-bypass stale feedback budget-reset long-run generalization as current representative closeout

## Follow-Up

- evaluate broader redundancy stability variants beyond the current flap-bypass representative wrapper set
- evaluate threshold-family growth and future redundancy mode growth outside the current active-standby envelope
