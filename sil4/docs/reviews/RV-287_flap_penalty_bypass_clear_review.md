# Review Record - Flap-Penalty Bypass Clear

## Document Control

- Review ID: `RV-287`
- Date: `2026-05-01`
- Author: `Codex`
- Scope: `R-003 flap-penalty bypass-clear telemetry`

## Reviewed Items

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

1. armed flap penalty가 active-loss bypass로 clear될 때 ordinary clear count를 오염시키지 않는지 점검한다.
2. bypass clear count가 pending penalty가 실제 bypass path에서 해제될 때만 증가하는지 점검한다.
3. switch-audit bypass reason과 penalty clear telemetry가 같은 representative flow를 가리키는지 확인한다.

## Findings

1. preferred flap-reset 뒤 renewed hold 상태에서 active secondary loss가 발생하면 preferred primary bypass recovery가 선택되고 pending penalty는 즉시 clear된다.
2. 이 경로에서는 ordinary clear count는 유지되고 bypass-clear count만 `0 -> 1`로 증가하므로 completion clear와 bypass clear를 분리 audit할 수 있다.
3. `TC-CHM-054`, `TC-SUP-066`, `TC-INT-202`는 같은 bypass-clear representative flow를 unit/integration/spec/traceability 기준으로 일관되게 고정한다.

## Conclusion

- flap-penalty clear telemetry가 ordinary completion clear와 bypass clear로 분리됐다.
- switch-audit bypass path도 penalty lifecycle 관점에서 caller-visible observability를 갖게 됐다.
- `R-003` residual은 계속 current family 내부 숫자 확장보다 richer redundancy-policy observability/generalization 방향으로 좁혀진다.
