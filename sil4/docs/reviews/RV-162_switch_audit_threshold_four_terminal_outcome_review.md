# RV-162 Switch Audit Threshold Four Terminal Outcome Review

## Scope
- threshold-aware terminal outcome family를 holdoff `4` representative flow까지 확장한다.

## Observation
- current terminal outcome family는 mixed long-run cumulative parity와 threshold `3` representative flow까지는 확보됐다.
- 하지만 threshold-aware terminal outcome representative path를 한 단계 더 올려야 current family 밖의 next policy growth와 current-family parity를 더 분리해서 읽을 수 있다.

## Judgment
- holdoff `4` ordinary complete, abort, bypass complete cumulative parity를 unit/integration 둘 다 고정하면 threshold-aware terminal outcome family가 current representative policy 범위 안에서 더 안정적으로 닫힌다.

## Decision
- `TC-SUP-045`, `TC-INT-163`를 추가한다.
- representative flow는
  - `failover -> hold -> hold -> hold -> recovery`
  - `failover -> hold -> reset`
  - `failover -> hold -> bypass complete`
  순서로 실행한다.

## Result
- threshold `4`에서도 terminal outcome family는
  - ordinary complete
  - abort
  - bypass complete
  cumulative parity를 유지한다.
- holdoff progress/remaining은 threshold-aware policy와 모순 없이 누적되고 final state는 `ESTABLISHED`를 유지한다.
