# Review Record - Pending Flap-Penalty Telemetry

## Document Control

- Review ID: `RV-284`
- Date: `2026-04-28`
- Author: `Codex`
- Scope: `R-003 pending flap-penalty telemetry observability`

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

1. pending flap penalty가 없을 때 새 telemetry가 기존 holdoff contract를 오염시키지 않는지 점검한다.
2. preferred channel flap-down 이후 다음 recovery cycle에만 적용되는 pending penalty가 selection result와 supervisor report에 동일하게 보이는지 점검한다.
3. preferred recovery 완료 후 pending penalty가 clear되어 base holdoff telemetry로 복귀하는지 점검한다.

## Findings

1. channel manager selection result는 preferred recovery first hold에서 pending penalty `0`을, flap reset 직후와 renewed hold 동안 pending penalty `1`을, final recovery 뒤에는 다시 `0`을 보고한다.
2. supervisor report는 같은 sequence에서 pending penalty를 channel-manager runtime state와 동일하게 반영하므로, effective target `2 -> 3 -> 2` 전이를 private context 추론 없이 직접 audit할 수 있다.
3. 기존 flap-penalty 정책 자체는 변하지 않았고, 이번 단계는 armed/cleared 상태를 caller-visible telemetry로 드러내는 observability refinement에 해당한다.

## Conclusion

- pending flap-penalty state가 channel-manager와 supervisor 양쪽에서 명시적으로 관찰 가능해졌다.
- `TC-CHM-053`, `TC-SUP-065`, `TC-INT-201`은 flap-penalty policy와 pending/cleared telemetry를 함께 검증한다.
- `R-003` residual은 numeric parity가 아니라 richer redundancy-policy observability/generalization 쪽으로 더 일관되게 유지된다.
