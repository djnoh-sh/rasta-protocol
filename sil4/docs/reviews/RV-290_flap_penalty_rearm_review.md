# Review Record - Flap Penalty Rearm Telemetry

## Document Control

- Review ID: `RV-290`
- Scope: `preferred recovery flap-penalty rearm telemetry`
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

- penalty arm count만으로는 구분되지 않던 first arm과 abort-after-applied rearm을 분리할 수 있는지
- rearm telemetry가 abort telemetry와 중복되지 않고 함께 해석 가능한지
- channel-manager, supervisor, integration 관찰점이 같은 rearm progression을 유지하는지

## Findings

- `uPreferredRecoveryPenaltyRearmCount`는 existing pending penalty가 있는 상태에서 preferred channel이 다시 down되어 다음 cycle로 재무장될 때만 증가하므로 first arm과 rearm을 구분한다.
- abort count는 적용 중 cycle 중단을, rearm count는 그 결과 다음 cycle용 penalty가 다시 arm됐음을 뜻하므로 의미가 중복되지 않는다.
- unit, supervisor, integration test가 동일한 `arm 1 -> applied 1 -> abort 1 + rearm 1 + arm 2` progression을 검증해 low-level selection과 switch-audit report의 해석이 일치한다.

## Conclusion

- flap-penalty observability는 `arm -> rearm -> applied -> abort / ordinary clear / bypass clear`까지 더 세밀하게 해석 가능해졌다.
- current change는 `R-003`를 숫자 확장이 아니라 switch-audit semantics refinement 방향으로 더 좁히는 단계다.
