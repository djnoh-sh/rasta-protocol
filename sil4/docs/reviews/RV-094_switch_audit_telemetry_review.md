# RV-094 Switch Audit Telemetry Review

- Scope: `channel switch from/to audit telemetry`
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

- existing `uChannelSwitchCount` / `uLastChannelSwitchOccurred` telemetry만으로는 switching audit trace가 부족한지 점검한다.
- failover/recovery switch에서는 `from/to` channel이 보고되고 no-op refresh에서는 `INVALID`로 비워지는지 확인한다.

## Findings

1. supervisor report에 `eLastSwitchFromChannelId`, `eLastSwitchToChannelId`를 추가해 마지막 actual switch edge를 직접 노출한다.
2. failover/recovery switch에서는 `PRIMARY->SECONDARY`, `SECONDARY->PRIMARY`가 정확히 남는다.
3. no-op refresh에서는 switch flag가 `0`이고 `from/to`도 `INVALID`로 비워져 audit noise를 만들지 않는다.

## Decision

- switching audit policy의 observability는 한 단계 강화됐다.
- 다음 `R-003` residual은 telemetry 부재보다 richer policy 자체에 더 가깝다.
