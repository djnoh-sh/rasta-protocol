# RV-253 Flap-Bypass Stale Mixed Feedback Budget-Reset Long-Run Review

## Scope

- add long-run integration coverage that combines flap-bypass stale completion/failure filtering with correlated send/receive budget reset in one representative path
- align feedback representative wrapper, flap-bypass closeout wrapper, roadmap, spec, and traceability wording to that new long-run state

## Inputs Reviewed

- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-INT-193` shows that inactive-channel stale `SEND_COMPLETED` and `SEND_FAILED` events remain filtered across repeated flap-bypass cycles while active primary `SEND_FAILED` and `RX_ERROR` still contribute to local budgets.
2. Each cycle resets both send-failure and receive-error budgets through the same-cycle primary success, and the second cycle reconstructs the same stale-mixed-feedback plus budget-reset pattern without contradiction.
3. `TC-INT-172` and `TC-INT-111` now describe the flap-bypass long-run family as including this combined stale-mixed-feedback budget-reset generalization rather than splitting it across narrower paths only.

## Decision

- pass
- treat flap-bypass stale mixed feedback budget-reset long-run generalization as current representative closeout

## Follow-Up

- evaluate broader redundancy stability variants beyond the current flap-bypass representative wrapper set
- evaluate threshold-family growth and future redundancy mode growth outside the current active-standby envelope
