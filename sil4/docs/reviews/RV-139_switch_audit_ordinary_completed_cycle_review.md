# RV-139 Switch Audit Ordinary Completed Cycle Review

## Scope
- completed holdoff cycle cumulative telemetry 안에서 ordinary completed cycle을 직접 노출한다.

## Observation
- current switch audit는
  - `uCompletedHoldoffCycleCount`
  - `uBypassCompletedHoldoffCycleCount`
  - `uAbortedHoldoffCycleCount`
  를 이미 가진다.
- 따라서 ordinary completed cycle은 `completed - bypass-completed`로 유도 가능하지만, direct audit 관점에서는 해석 단계를 거쳐야 한다.

## Judgment
- holdoff outcome subtype을 direct bucket으로 모두 관찰하게 만들어야 current holdoff-outcome-aware envelope가 더 명료하다.

## Decision
- `uCompletedHoldoffCycleCount`는 broad completed bucket으로 유지한다.
- `uOrdinaryCompletedHoldoffCycleCount`를 추가해 ordinary holdoff-complete cycle을 직접 누적한다.
- bypass-completed cycle은 `uBypassCompletedHoldoffCycleCount`로 계속 분리한다.
- unit/integration에서 ordinary path와 bypass path를 각각 검증한다.

## Result
- holdoff cycle outcome cumulative observability는
  - ordinary completed
  - bypass completed
  - aborted
  를 direct bucket으로 갖게 된다.
- current switch audit family는 holdoff outcome subtype parity 관점에서도 representative closeout 상태에 더 가까워진다.
