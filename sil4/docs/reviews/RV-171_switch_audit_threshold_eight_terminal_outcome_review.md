# RV-171 Switch Audit Threshold Eight Terminal Outcome Review

## Scope
- `holdoff=8`에서도 threshold-aware terminal outcome mixed long-run representative flow가 유지되는지 unit/integration 기준으로 고정한다.

## Observation
- current terminal outcome family는 mixed ordinary complete, abort/reset, bypass complete cumulative parity와 post-terminal retention parity까지 representative closeout 상태다.
- 다만 threshold-aware terminal outcome representative flow는 현재 `3`부터 `7`까지만 direct coverage가 있고, `8`은 current family generalization의 다음 인접 단계다.

## Judgment
- `holdoff=8`은 새 policy family가 아니라 current threshold-aware terminal outcome family의 자연스러운 확장이다.
- 따라서 ordinary completion이 7회의 hold 뒤에만 허용되고, 같은 run 안의 abort/reset과 bypass complete가 cumulative parity를 깨지 않는지만 direct representative flow로 추가하면 충분하다.

## Decision
- 아래 representative 항목을 추가한다.
  - `TC-SUP-050` threshold-eight terminal outcome matrix
  - `TC-INT-168` threshold-eight terminal outcome integration
- expected path는 아래와 같다.
  - `failover -> hold x7 -> ordinary completion`
  - `failover -> hold -> abort reset`
  - `renewed hold -> bypass completion`

## Result
- current switch audit family는 threshold-aware terminal outcome representative flow를 `3/4/5/6/7/8`까지 직접 확보한다.
- `R-003` residual은 current threshold-eight parity 부족이 아니라 그보다 바깥의 threshold family generalization과 broader policy growth 쪽으로 더 좁아진다.
