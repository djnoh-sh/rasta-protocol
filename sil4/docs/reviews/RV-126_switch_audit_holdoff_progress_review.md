# RV-126 Switch Audit Holdoff Progress Review

## Summary
- switch audit telemetry에 holdoff 진행도를 추가했다.
- `holdoff no-op`가 발생했을 때 단순히 no-op reason만 남는 것이 아니라, 현재 stable-selection progress와 configured holdoff target도 supervisor report에서 직접 관찰 가능해졌다.

## Added Observability
- `uPreferredRecoveryHoldoffProgressCount`
- `uPreferredRecoveryHoldoffTargetCount`

## Covered Contract
- holdoff no-op:
  - progress increments
- preferred recovery switch:
  - progress resets to `0`
- already-active repeated refresh:
  - progress remains `0`
- target:
  - remains equal to configured holdoff selection count

## Coverage
- `TC-SUP-038` switch audit holdoff progress matrix
- `TC-INT-156` switch audit holdoff progress integration

## Result
- current switch audit family is now covered through:
  - `kind`
  - `from/to`
  - `reason`
  - trigger channel/event
  - cumulative counts
  - holdoff progress/target

## Residual
- current active-standby holdoff audit family gap is narrower again.
- remaining work is future audit-policy growth beyond the current trigger-aware, holdoff-progress-aware envelope.
