# RV-143 Switch Audit Terminal Holdoff Outcome Review

## Scope
- last-event 관점에서 마지막 terminal holdoff outcome을 direct taxonomy로 고정한다.

## Observation
- current switch audit는
  - `eLastHoldoffCycleState`
  - `eLastCompletedHoldoffCycleKind`
  를 함께 보면 마지막 outcome을 유도할 수 있다.
- 하지만 terminal outcome을 읽으려면 state와 completed kind를 결합 해석해야 해서 audit 해상도가 직접적이지 않다.

## Judgment
- holdoff outcome audit가 cumulative split과 last completed subtype까지 가진 상태라면, 마지막 terminal outcome도 direct taxonomy로 노출하는 편이 더 일관된다.

## Decision
- `eLastTerminalHoldoffOutcome`를 추가한다.
- 값:
  - `NONE`
  - `ORDINARY_COMPLETED`
  - `BYPASS_COMPLETED`
  - `ABORTED`
- ordinary completed, bypass completed, abort path와 repeated refresh retention path를 unit/integration에서 검증한다.

## Result
- holdoff outcome audit는 이제
  - cumulative bucket
  - last completed subtype
  - last terminal outcome
  을 모두 direct field로 제공한다.
- current switch audit family는 holdoff outcome observability 관점에서 representative closeout 상태에 더 가까워진다.
