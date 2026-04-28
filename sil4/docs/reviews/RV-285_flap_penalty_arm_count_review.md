# Review Record - Flap-Penalty Arm Count

## Document Control

- Review ID: `RV-285`
- Date: `2026-04-29`
- Author: `Codex`
- Scope: `R-003 flap-penalty arm-count telemetry`

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

1. preferred flap-reset path가 penalty를 arm할 때 누적 arm count를 정확히 1회만 증가시키는지 점검한다.
2. armed 상태가 renewed hold와 final recovery를 거쳐도 cumulative arm count가 보존되는지 점검한다.
3. 새 cumulative telemetry가 기존 pending/target/remaining contract와 충돌하지 않는지 점검한다.

## Findings

1. preferred holdoff progress가 있던 상태에서 preferred channel이 다시 down되면 pending penalty가 arm되고, 같은 순간 cumulative penalty-arm count도 `0 -> 1`로 증가한다.
2. renewed hold 동안 pending penalty는 유지되고 final preferred recovery 후 pending state는 clear되지만 arm count는 cumulative audit 값으로 retain된다.
3. 이번 단계는 flap-penalty semantics 자체를 바꾸지 않고, repeated flap behavior를 long-run audit 관점에서 더 직접 추적할 수 있게 한다.

## Conclusion

- cumulative flap-penalty arm count가 channel-manager와 supervisor report 양쪽에서 관찰 가능해졌다.
- `TC-CHM-053`, `TC-SUP-065`, `TC-INT-201`은 pending state뿐 아니라 cumulative arm history도 함께 고정한다.
- `R-003` residual은 숫자 확대보다 richer redundancy observability/generalization 방향으로 계속 정리된다.
