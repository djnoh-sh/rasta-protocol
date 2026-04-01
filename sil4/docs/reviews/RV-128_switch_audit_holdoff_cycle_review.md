# RV-128 Switch Audit Holdoff Cycle Review

## Summary
- switch audit telemetry에 holdoff cycle 관찰값을 추가했다.
- holdoff no-op의 첫 진입과 reset 뒤 renewed hold를 구분해 누적 관찰할 수 있게 됐다.

## Added Observability
- `uHoldoffCycleCount`

## Covered Contract
- first holdoff no-op in a cycle:
  - cycle count increments
- continued hold within the same cycle:
  - cycle count does not increment again
- preferred recovery:
  - cycle count is retained
- holdoff reset followed by renewed hold:
  - renewed hold starts a new cycle and increments cycle count

## Coverage
- `TC-SUP-040` switch audit holdoff cycle matrix
- `TC-INT-158` switch audit holdoff cycle integration

## Result
- current switch audit family is now covered through:
  - `kind`
  - `from/to`
  - `reason`
  - trigger channel/event
  - cumulative counts
  - holdoff progress/target/remaining
  - holdoff reset reason/count
  - holdoff cycle count

## Residual
- current active-standby holdoff audit family gap is narrower again.
- remaining work is future audit-policy growth beyond the current trigger-aware, holdoff-progress/remaining/reset/cycle-aware envelope.
