# RV-132 Switch Audit No-Op Origin Review

## Summary
- switch audit의 no-op refresh를 trigger origin 기준으로도 누적 분리했다.
- 이제 no-op refresh가 preferred channel refresh에서 왔는지, non-preferred channel refresh에서 왔는지를 supervisor report에서 cumulative count로 직접 구분할 수 있다.

## Added Observability
- `uPreferredChannelTriggeredNoOpRefreshCount`
- `uNonPreferredChannelTriggeredNoOpRefreshCount`

## Covered Contract
- preferred channel `CHANNEL_UP`로 발생한 no-op:
  - `uPreferredChannelTriggeredNoOpRefreshCount`
- non-preferred channel `CHANNEL_UP`로 발생한 no-op:
  - `uNonPreferredChannelTriggeredNoOpRefreshCount`
- no-op reason split:
  - `HOLDOFF_REFRESH_NOOP`
  - `ACTIVE_REFRESH_NOOP`
  와 독립적으로 origin count가 누적된다.

## Coverage
- `TC-SUP-035` switch audit cumulative matrix
- `TC-SUP-036` switch audit envelope matrix
- `TC-SUP-037` switch audit reason matrix
- `TC-INT-131` switch audit cumulative integration
- `TC-INT-134` switch audit envelope integration
- `TC-INT-155` switch audit reason integration

## Result
- current switch audit family is now covered through:
  - `kind`
  - `from/to`
  - `reason`
  - trigger channel/event
  - cumulative switch counts
  - preferred-vs-non-preferred no-op origin counts
  - holdoff progress/target/remaining
  - holdoff reset reason/count
  - holdoff cycle/completed-cycle/aborted-cycle count
  - last holdoff cycle state
  - immediate-vs-after-holdoff preferred recovery split
  - immediate-vs-after-holdoff preferred recovery cumulative count split

## Residual
- current active-standby holdoff audit family gap is narrower again.
- remaining work is future audit-policy growth beyond the current trigger-aware, no-op-origin-aware, holdoff-progress/remaining/reset/cycle/completion/abort/last-state-aware envelope.
