# RV-165 Switch Audit Threshold Seven Terminal Outcome Review

## Scope
- threshold-aware terminal outcome family를 holdoff `7` representative flow까지 확장한다.

## Observation
- current terminal outcome family는 mixed long-run cumulative parity와 threshold `3`, `4`, `5`, `6` representative flow까지는 확보됐다.
- threshold-aware terminal outcome current family를 더 안정적으로 닫으려면 holdoff `7` representative path도 unit/integration 기준으로 같이 고정하는 편이 맞다.

## Judgment
- holdoff `7` ordinary complete, abort, bypass complete cumulative parity를 unit/integration 둘 다 고정하면 current threshold-aware terminal outcome family와 future policy growth를 더 깔끔하게 분리할 수 있다.

## Decision
- `TC-SUP-048`, `TC-INT-166`를 추가한다.
- representative flow는
  - `failover -> hold -> hold -> hold -> hold -> hold -> hold -> recovery`
  - `failover -> hold -> reset`
  - `failover -> hold -> bypass complete`
  순서로 실행한다.

## Result
- threshold `7`에서도 terminal outcome family는
  - ordinary complete
  - abort
  - bypass complete
  cumulative parity를 유지한다.
- holdoff progress/remaining은 threshold-aware policy와 모순 없이 누적되고 final state는 `ESTABLISHED`를 유지한다.
