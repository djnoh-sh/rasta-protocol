# RV-121 Switch Audit Reason Review

## Summary
- switch audit telemetry를 `kind/from/to`와 coarse cumulative count에서 한 단계 더 확장했다.
- no-op refresh를 `holdoff refresh noop`와 `active refresh noop`로 구분하고, last reason과 cumulative count를 supervisor report에 추가했다.

## Added Coverage
- `TC-SUP-037` switch audit reason matrix
- `TC-INT-155` switch audit reason integration

## Fixed Policy
- failover actual switch:
  - `RSRX_SUPERVISOR_SWITCH_REASON_FAILOVER_CHANNEL_DOWN`
- preferred recovery actual switch:
  - `RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_COMPLETED`
- preferred primary가 restored되었지만 holdoff 때문에 secondary를 유지한 refresh:
  - `RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_REFRESH_NOOP`
- 이미 preferred channel이 active인 상태에서 repeated refresh로 no switch가 발생한 경우:
  - `RSRX_SUPERVISOR_SWITCH_REASON_ACTIVE_REFRESH_NOOP`

## Observability
- last-event reason:
  - `eLastSwitchReason`
- cumulative no-op subtype counts:
  - `uHoldoffRefreshNoOpCount`
  - `uActiveRefreshNoOpCount`

## Result
- current switch audit envelope는 이제
  - `kind`
  - `from/to`
  - actual-switch cumulative counts
  - no-op subtype reason and cumulative counts
  까지 representative coverage를 가진다.
- residual은 current taxonomy 내부 parity보다 future audit-policy growth로 좁혀진다.
