# RV-146 Switch Audit Terminal Outcome Count Review

## Scope
- terminal holdoff outcome subtype cumulative count parity를 supervisor report와 representative unit/integration flow에 추가한다.

## Observation
- 기존 telemetry는 `eLastTerminalHoldoffOutcome`와 trigger를 통해 마지막 terminal holdoff outcome의 종류와 원인은 직접 관찰할 수 있었다.
- 하지만 ordinary completed, bypass completed, aborted terminal outcome의 누적 빈도는 direct field 없이 cycle count와 last outcome 조합으로만 간접 추론해야 했다.

## Judgment
- current switch audit family가 holdoff outcome subtype parity를 representative closeout 수준으로 유지하려면 terminal outcome taxonomy도 last-event뿐 아니라 cumulative subtype count를 직접 노출하는 편이 맞다.
- 이 count는 기존 holdoff cycle count와 중복이 아니라 terminal outcome taxonomy 관찰 해상도를 올리는 별도 축이다.

## Decision
- 아래 cumulative field를 추가한다.
  - `uOrdinaryTerminalHoldoffOutcomeCount`
  - `uBypassTerminalHoldoffOutcomeCount`
  - `uAbortedTerminalHoldoffOutcomeCount`
- ordinary complete, bypass complete, abort path에서만 각 subtype count를 증가시킨다.
- repeated refresh와 unrelated event는 terminal outcome cumulative count를 오염시키지 않는다.

## Result
- switch audit holdoff outcome wrapper는 이제
  - cycle outcome cumulative count
  - last completed subtype
  - last terminal outcome
  - last terminal outcome trigger
  - terminal outcome subtype cumulative count
  를 함께 검증한다.
- current switch audit family는 terminal-outcome-count-aware envelope 기준으로 더 좁혀진다.
