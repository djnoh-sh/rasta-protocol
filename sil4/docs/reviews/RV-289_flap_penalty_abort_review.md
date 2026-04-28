# Review Record - Flap Penalty Abort Telemetry

## Document Control

- Review ID: `RV-289`
- Scope: `preferred recovery flap-penalty abort telemetry`
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

- applied penalty holdoff cycle이 preferred 재-flap-down으로 다시 중단될 때 abort telemetry가 분리 집계되는지
- arm/applied/abort/ordinary-clear/bypass-clear 누적 chain이 서로 중복 없이 해석 가능한지
- channel-manager, supervisor, integration 관찰점이 같은 abort progression을 유지하는지

## Findings

- `uPreferredRecoveryPenaltyAbortCount`는 pending penalty가 실제 holdoff에 적용된 뒤 preferred channel이 다시 down된 경우에만 증가하므로 arm-only flap과 applied-cycle abort를 구분한다.
- abort 시 pending penalty는 다음 recovery cycle로 유지되고 applied count는 보존되므로 “적용은 시작됐지만 완료되진 않았다”는 상태를 누적으로 해석할 수 있다.
- unit, supervisor, integration test가 동일한 `arm 1 -> applied 1 -> abort 1 -> arm 2` progression을 검증하므로 low-level selection과 switch-audit report 해석이 일치한다.

## Conclusion

- flap-penalty observability는 `arm -> applied -> abort / ordinary clear / bypass clear` chain까지 분리돼 현재 representative closeout 범위 안에서 해석 가능성이 더 높아졌다.
- current change는 `R-003`을 추가 숫자 증가가 아닌 audit semantics refinement 방향으로 더 좁히는 데 기여한다.
