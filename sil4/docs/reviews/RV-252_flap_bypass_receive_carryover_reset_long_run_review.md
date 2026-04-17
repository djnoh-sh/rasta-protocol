# RV-252 Flap-Bypass Receive Carryover Reset Long-Run Review

## Scope

- add long-run integration coverage that combines flap-bypass receive budget reset and later carryover escalation in one representative path
- align receive representative wrapper, roadmap, spec, and traceability wording to that new long-run state

## Inputs Reviewed

- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-INT-192` shows that a first flap-bypass cycle can reset a carried receive-error budget via primary success and that a later flap-bypass cycle can rebuild the same carryover and escalate on the next primary `RX_ERROR`.
2. `TC-INT-180` and `TC-INT-171` now describe flap-bypass receive representative coverage as including both single-cycle carryover/reset paths and the longer carryover-reset generalization path.
3. current `R-003` wording is narrower because flap-bypass receive residual is no longer just single-cycle representative evidence.

## Decision

- pass
- treat flap-bypass receive carryover/reset long-run generalization as current representative closeout

## Follow-Up

- evaluate broader redundancy mode growth beyond current active-standby flap-bypass family
- evaluate longer-run stability variants outside the current representative wrapper set
