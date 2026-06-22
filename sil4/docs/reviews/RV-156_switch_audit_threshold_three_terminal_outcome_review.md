# RV-156 Switch Audit Threshold-Three Terminal Outcome Review

## Scope
- holdoff threshold `3`와 terminal outcome mixed long-run parity를 결합한 representative path를 추가한다.

## Observation
- current terminal outcome family는 holdoff `2` 기준 mixed long-run에서 ordinary complete, abort, bypass complete cumulative parity까지는 닫혀 있다.
- 하지만 higher-threshold policy와 terminal outcome family의 결합은 current representative flow에서 직접 보이지 않았다.

## Judgment
- broader longer-run stability envelope를 더 직접 넓히려면 새로운 telemetry를 더 쪼개기보다, existing terminal outcome family가 threshold growth와 함께도 모순 없이 유지된다는 representative path가 먼저 필요하다.

## Decision
- unit:
  - `TC-SUP-044` threshold-three mixed long-run matrix 추가
- integration:
  - `TC-INT-162` threshold-three mixed long-run integration 추가
- terminal outcome envelope wrapper는 기존 mixed long-run path와 함께 threshold-three mixed long-run path도 포함하도록 확장한다.

## Result
- current switch audit family는 holdoff threshold `3`에서도
  - ordinary completed
  - aborted
  - bypass completed
  cumulative parity를 representative flow 수준에서 유지한다.
- `R-003` residual은 current threshold-aware terminal outcome parity 부족보다 next policy growth와 broader stability envelope 쪽으로 더 좁아진다.
