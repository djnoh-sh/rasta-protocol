# Review Record - Flap-Penalty Clear Count

## Document Control

- Review ID: `RV-286`
- Date: `2026-04-30`
- Author: `Codex`
- Scope: `R-003 flap-penalty clear-count telemetry`

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

1. pending flap penalty가 실제 preferred recovery completion에서만 clear count로 누적되는지 점검한다.
2. arm 이후 renewed hold 동안 clear count가 premature increment 되지 않는지 확인한다.
3. final preferred recovery 뒤 pending state는 clear되고 cumulative clear count는 retain되는지 점검한다.

## Findings

1. preferred flap-down으로 pending penalty가 armed된 뒤 renewed hold 단계에서는 clear count가 계속 `0`으로 유지된다.
2. final preferred recovery가 완료되는 순간 pending penalty는 `0`으로 clear되고 cumulative clear count는 `1`로 증가한다.
3. 이번 단계는 flap-penalty policy를 바꾸지 않고, arm된 penalty cycle이 실제 completion까지 도달했는지 caller-visible telemetry로 분리해준다.

## Conclusion

- cumulative flap-penalty clear count가 channel-manager와 supervisor report 양쪽에서 관찰 가능해졌다.
- `TC-CHM-053`, `TC-SUP-065`, `TC-INT-201`은 arm history뿐 아니라 completed penalty-cycle history도 함께 고정한다.
- `R-003` residual은 계속 숫자 증가가 아니라 richer redundancy observability/generalization 방향으로 정리된다.
