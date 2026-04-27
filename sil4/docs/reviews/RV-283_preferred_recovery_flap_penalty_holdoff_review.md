# Review Record - Preferred Recovery Flap-Penalty Holdoff

## Document Control

- Review ID: `RV-283`
- Date: `2026-04-27`
- Author: `Codex`
- Scope: `R-003 preferred recovery flap-penalty holdoff policy`

## Reviewed Items

- `sil4/include/rsrx_channel_manager.h`
- `sil4/src/rsrx_channel_manager.c`
- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/unit/test_rsrx_transport_supervisor.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/design/lld/channel_manager_lld_draft.md`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Review Focus

1. configured flap penalty가 `0`일 때 기존 preferred recovery holdoff/flap-reset contract를 바꾸지 않는지 점검한다.
2. holdoff 도중 preferred channel flap-down 이후 다음 recovery cycle의 effective target이 deterministic하게 증가하는지 점검한다.
3. channel-manager selection result와 supervisor/integration telemetry가 같은 penalty-adjusted target/remaining 값을 보고하는지 점검한다.

## Findings

1. channel manager는 pending flap penalty를 runtime context에만 보관하고, configured `uPreferredRecoveryFlapPenaltySelections == 0`이면 기존 holdoff target 계산을 그대로 유지한다.
2. holdoff progress가 누적된 상태에서 preferred channel이 다시 down되면 current cycle progress는 reset되고, 다음 recovery cycle의 effective target은 `base + configured penalty`로 계산된다.
3. preferred recovery가 실제 완료되거나 active-loss bypass로 preferred channel이 선택되면 pending penalty는 clear되어 target/remaining telemetry가 base holdoff로 되돌아간다.
4. `TC-CHM-053`, `TC-SUP-065`, `TC-INT-201`은 같은 flap-penalty path를 unit/integration/spec/traceability 기준으로 일관되게 가리킨다.

## Conclusion

- preferred recovery flap-reset representative closeout은 유지된다.
- configured flap-penalty holdoff path가 next-step redundancy policy growth로 추가됐다.
- `R-003` residual은 추가 숫자 증가보다 richer suppression/generalization policy 쪽으로 더 좁혀졌다.
