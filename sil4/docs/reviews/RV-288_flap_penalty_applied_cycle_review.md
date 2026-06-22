# Review Record - Flap Penalty Applied Cycle Telemetry

## Document Control

- Review ID: `RV-288`
- Scope: `preferred recovery flap-penalty applied cycle telemetry`
- Status: `Accepted`
- Reviewer: `Codex`
- Review Date: `2026-04-28`

## Reviewed Artifacts

- `sil4/include/rsrx_channel_manager.h`
- `sil4/include/rsrx_transport_supervisor.h`
- `sil4/src/rsrx_channel_manager.c`
- `sil4/src/rsrx_transport_supervisor.c`
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

- pending flap penalty가 실제 restored-again holdoff cycle에 적용되기 시작한 시점을 누적으로 분리 보고하는지
- arm/applied/ordinary-clear/bypass-clear telemetry가 서로 중복 없이 해석 가능한지
- channel-manager 결과와 supervisor report, integration 관찰점이 같은 count를 유지하는지

## Findings

- `uPreferredRecoveryPenaltyAppliedCycleCount`는 pending penalty가 존재하는 상태에서 renewed holdoff cycle의 첫 stable select가 시작될 때만 증가하도록 배치돼, arm-only 상태와 actual-applied 상태를 분리한다.
- ordinary completion과 bypass completion은 기존 clear telemetry를 유지하고, 새 applied-cycle count는 holdoff cycle start audit만 담당하므로 의미 중복이 없다.
- unit, supervisor, integration test가 같은 `0 -> 1 -> 1` progression을 검증하므로 low-level selection과 higher-level switch-audit report 사이의 cross-check 경로가 유지된다.

## Conclusion

- flap-penalty observability는 `arm -> applied -> ordinary clear / bypass clear`의 누적 chain으로 더 명확해졌다.
- current change는 `R-003` representative closeout 범위를 유지하면서 next redundancy policy growth residual만 남기는 방향과 정합하다.
