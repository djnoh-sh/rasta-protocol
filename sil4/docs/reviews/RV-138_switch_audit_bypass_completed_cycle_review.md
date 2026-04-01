# RV-138 Switch Audit Bypass Completed Cycle Review

## Scope
- completed holdoff cycle cumulative telemetry 안에서 bypass-completed cycle을 별도 관찰 가능하게 만든다.

## Observation
- current switch audit는
  - `uCompletedHoldoffCycleCount`
  - `uAbortedHoldoffCycleCount`
  - `uCompletedHoldoffPreferredRecoverySwitchCount`
  - `uBypassPreferredRecoverySwitchCount`
  를 이미 노출한다.
- 하지만 holdoff cycle outcome 자체는 ordinary holdoff-complete recovery와 bypass recovery가 모두 `uCompletedHoldoffCycleCount` 안에 묶여 있어 cumulative cycle outcome 관점에서는 직접 분리되지 않았다.

## Judgment
- switch count 기준의 ordinary-vs-bypass split을 holdoff cycle outcome까지 올려야 current holdoff-outcome-aware envelope가 더 일관된다.

## Decision
- `uCompletedHoldoffCycleCount`는 broad completed cycle bucket으로 유지한다.
- `uBypassCompletedHoldoffCycleCount`를 추가해 bypass로 닫힌 completed cycle을 별도 누적한다.
- ordinary holdoff-complete cycle은 broad completed count와 bypass-completed count의 차이로 해석 가능하게 둔다.
- unit/integration bypass path에서 new cumulative bucket을 직접 검증한다.

## Result
- holdoff cycle outcome cumulative observability는
  - completed
  - bypass-completed
  - aborted
  세 축으로 더 선명해진다.
- current switch audit family는 holdoff outcome subtype 관점에서도 representative closeout 상태에 더 가까워진다.
