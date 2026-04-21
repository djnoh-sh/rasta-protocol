# RV-259 Preferred Recovery Flap Reset Threshold Nineteen Review

## Scope

- extend preferred recovery flap-reset representative parity from `2..18` to `2..19`
- align channel-manager unit parity, session-supervisor integration parity, traceability, and roadmap wording to the same threshold envelope
- keep direct switch-audit terminal outcome coverage unchanged at `3..19`

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `TC-CHM-046` adds direct channel-manager pure holdoff threshold `19` parity so unit traceability matches the existing integration and roadmap envelope.
2. `TC-CHM-047` adds direct channel-manager flap-reset threshold `19` parity.
3. `TC-INT-198` verifies integration-side flap-reset threshold `19`: first hold is reset by primary flap down, then eighteen renewed stable preferred-channel signals hold secondary active before the nineteenth signal restores primary.
4. The loop-based assertions reduce copied assertion drift while preserving selected-channel, switch-count, progress/remaining, final `ESTABLISHED`, and application callback checks.
5. Current preferred recovery holdoff/flap-reset parity now reads as `2..19`; residual redundancy work moves beyond the current threshold `19` envelope.

## Decision

- Pass.
- Treat preferred recovery holdoff/flap-reset representative coverage as current through `2..19`.

## Follow-Up

- evaluate preferred recovery and terminal outcome threshold-family growth beyond current `19`
- evaluate broader redundancy policy growth outside the current switch-audit envelope
