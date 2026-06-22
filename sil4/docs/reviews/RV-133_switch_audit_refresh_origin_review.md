# RV-133 Switch Audit Refresh Origin Review

## Summary
- switch audit에 `CHANNEL_UP` 기반 refresh 전체의 trigger origin cumulative count를 추가했다.
- 이제 no-op만이 아니라 preferred recovery switch까지 포함한 refresh event 전체를 preferred/non-preferred origin 기준으로 supervisor report에서 직접 구분할 수 있다.

## Added Observability
- `uPreferredChannelTriggeredRefreshEventCount`
- `uNonPreferredChannelTriggeredRefreshEventCount`

## Covered Contract
- preferred channel `CHANNEL_UP`:
  - preferred-triggered refresh-total count 증가
- non-preferred channel `CHANNEL_UP`:
  - non-preferred-triggered refresh-total count 증가
- refresh-total origin count는
  - preferred recovery switch
  - holdoff no-op
  - active repeated refresh no-op
  을 모두 포함한다.

## Coverage
- `TC-SUP-035` switch audit cumulative matrix
- `TC-INT-131` switch audit cumulative integration

## Result
- current switch audit family is now covered through:
  - `kind`
  - `from/to`
  - `reason`
  - trigger channel/event
  - cumulative switch counts
  - preferred-vs-non-preferred refresh-total origin counts
  - preferred-vs-non-preferred no-op origin counts
  - holdoff progress/target/remaining
  - holdoff reset reason/count
  - holdoff cycle/completed-cycle/aborted-cycle count
  - last holdoff cycle state
  - immediate-vs-after-holdoff preferred recovery split
  - immediate-vs-after-holdoff preferred recovery cumulative count split

## Residual
- current active-standby holdoff audit family gap is narrower again.
- remaining work is future audit-policy growth beyond the current trigger-aware, refresh-origin-aware, no-op-origin-aware, holdoff-progress/remaining/reset/cycle/completion/abort/last-state-aware envelope.
