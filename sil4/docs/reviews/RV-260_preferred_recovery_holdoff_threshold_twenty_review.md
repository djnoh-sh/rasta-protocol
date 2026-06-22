# RV-260 Preferred Recovery Holdoff Threshold Twenty Review

## Scope

- extend preferred recovery pure holdoff representative parity from `2..19` to `2..20`
- keep preferred recovery flap-reset parity unchanged at `2..19`
- keep direct switch-audit terminal outcome coverage unchanged at `3..19`

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-CHM-048` adds direct channel-manager pure holdoff threshold `20` parity.
2. `TC-INT-199` verifies integration-side holdoff threshold `20`: nineteen stable preferred-channel signals hold secondary active before the twentieth signal restores primary.
3. The threshold closeout wrappers now include pure holdoff `3..20` while preserving flap-reset `3..19`.
4. Roadmap wording separates pure holdoff `2..20`, flap-reset `2..19`, and terminal outcome `3..19`, keeping the next residual precise.

## Decision

- Pass.
- Treat preferred recovery pure holdoff representative coverage as current through `2..20`.

## Follow-Up

- Do not continue numeric threshold growth by default.
- Reopen preferred recovery flap-reset or terminal outcome threshold growth only when a new redundancy policy, configured limit, or failure-mode requirement needs evidence beyond the current representative range.
