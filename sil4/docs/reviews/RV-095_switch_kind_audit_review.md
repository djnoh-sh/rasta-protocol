# RV-095 Switch Kind Audit Review

- Scope: `switch kind audit telemetry`
- Date: `2026-03-27`
- Reviewer: `Project Team`

## Inputs Reviewed

- `sil4/include/rsrx_transport_supervisor.h`
- `sil4/src/rsrx_transport_supervisor.c`
- `sil4/tests/unit/test_rsrx_transport_supervisor.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
- `sil4/docs/roadmap_status.md`

## Review Focus

- 마지막 switch가 `failover`인지 `preferred recovery`인지 report에서 직접 구분되는지 점검한다.
- no-op refresh에서 switch audit telemetry가 비어 있는지 확인한다.

## Findings

1. report에 `eLastSwitchKind`를 추가해 `NONE`, `FAILOVER`, `PREFERRED_RECOVERY`를 직접 노출한다.
2. failover는 `FAILOVER + PRIMARY->SECONDARY`, recovery는 `PREFERRED_RECOVERY + SECONDARY->PRIMARY`로 구분된다.
3. no-op refresh는 `NONE + INVALID->INVALID`로 남아 audit noise를 만들지 않는다.

## Decision

- switching audit policy의 taxonomy observability는 representative 수준까지 닫혔다.
- 다음 `R-003` residual은 telemetry 분류보다 richer policy 자체에 더 가깝다.
