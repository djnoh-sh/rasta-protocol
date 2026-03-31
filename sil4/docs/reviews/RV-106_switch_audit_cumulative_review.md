# RV-106 Switch Audit Cumulative Review

## Scope

- `R-003` switch audit cumulative telemetry

## Findings

1. failover와 preferred recovery는 이제 last-switch taxonomy뿐 아니라 cumulative count로도 관찰된다.
2. no-op refresh는 `uFailoverSwitchCount`, `uPreferredRecoverySwitchCount`를 증가시키지 않아 audit noise를 만들지 않는다.
3. switch audit residual은 last-event observability 부족보다 broader audit policy growth 쪽으로 더 좁혀졌다.
