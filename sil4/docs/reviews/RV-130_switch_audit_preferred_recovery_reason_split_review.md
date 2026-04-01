# RV-130 Switch Audit Preferred Recovery Reason Split Review

## Summary
- switch audit의 preferred recovery reason을 immediate path와 after-holdoff path로 분리했다.
- recovery가 단순 preferred channel restore인지, holdoff cycle을 실제로 거쳐 닫힌 recovery인지 supervisor report에서 직접 구분할 수 있게 됐다.

## Added Observability
- `RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_IMMEDIATE`
- `RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_AFTER_HOLDOFF`
- `uImmediatePreferredRecoverySwitchCount`
- `uHoldoffPreferredRecoverySwitchCount`

## Covered Contract
- holdoff 없는 preferred recovery:
  - `PREFERRED_RECOVERY_IMMEDIATE`
- holdoff progress가 누적된 뒤 preferred recovery:
  - `PREFERRED_RECOVERY_AFTER_HOLDOFF`
- holdoff reset:
  - recovery reason으로 오염되지 않음

## Coverage
- `TC-SUP-037` switch audit reason matrix
- `TC-INT-155` switch audit reason integration

## Result
- current switch audit family is now covered through:
  - `kind`
  - `from/to`
  - `reason`
  - trigger channel/event
  - cumulative counts
  - holdoff progress/target/remaining
  - holdoff reset reason/count
  - holdoff cycle/completed-cycle count
- immediate-vs-after-holdoff preferred recovery split
- immediate-vs-after-holdoff preferred recovery cumulative count split

## Residual
- current active-standby holdoff audit family gap is narrower again.
- remaining work is future audit-policy growth beyond the current trigger-aware, holdoff-progress/remaining/reset/cycle/completion-aware envelope.
