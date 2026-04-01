# RV-124 Switch Audit Trigger Review

## Summary
- switch audit telemetry에 `trigger event type`과 `trigger channel id`를 추가했다.
- same `reason`이라도 어떤 runtime transport event가 그 audit record를 만들었는지 supervisor report에서 직접 확인 가능해졌다.

## Added Observability
- `eLastSwitchTriggerEventType`
- `eLastSwitchTriggerChannelId`

## Covered Paths
- failover:
  - `CHANNEL_DOWN(primary)`
- holdoff refresh no-op:
  - `CHANNEL_UP(primary)` while secondary retained
- preferred recovery:
  - `CHANNEL_UP(primary)` causing `SECONDARY -> PRIMARY`
- active repeated refresh no-op:
  - `CHANNEL_UP(primary)` while primary already active

## Result
- current switch audit family는 이제
  - `kind`
  - `from/to`
  - `reason`
  - cumulative counts
  - trigger event/channel
  까지 representative coverage를 가진다.

## Residual
- current-family observability gap은 더 줄었고,
- remaining gap is future audit-policy growth beyond the current trigger-aware envelope.
