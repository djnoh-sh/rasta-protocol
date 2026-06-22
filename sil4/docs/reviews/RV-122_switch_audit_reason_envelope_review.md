# RV-122 Switch Audit Reason Envelope Review

## Summary
- `switch audit envelope` wrapper가 기존 `closeout + cumulative`만 묶고 있어서, 새로 추가한 `reason` parity가 representative wrapper에 반영되지 않았다.
- 이번 정리로 envelope wrapper가 `reason`까지 포함하도록 맞췄다.

## Updated Wrapper Scope
- unit:
  - `TC-SUP-036` now includes `TC-SUP-037`
- integration:
  - `TC-INT-134` now includes `TC-INT-155`

## Result
- current switch audit envelope는 이제
  - `kind/from/to`
  - cumulative switch counts
  - holdoff-vs-active no-op reason parity
  를 하나의 representative wrapper에서 함께 추적한다.

## Residual
- current envelope 내부 parity gap은 줄었고,
- 남은 residual은 future switch audit policy growth 쪽으로 더 좁혀진다.
