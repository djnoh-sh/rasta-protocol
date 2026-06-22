# RV-147 Switch Audit Terminal Outcome Total Review

## Scope
- terminal holdoff outcome subtype cumulative count 위에 direct total cumulative count를 supervisor report와 representative flow에 추가한다.

## Observation
- current telemetry는 ordinary/bypass/aborted terminal outcome subtype count를 직접 노출하지만, 전체 terminal outcome 누적량은 세 count를 합산해야만 읽을 수 있다.
- representative review와 runtime audit 입장에서는 "terminal outcome이 총 몇 번 닫혔는가"를 direct field로 보는 편이 더 명확하다.

## Judgment
- `uTerminalHoldoffOutcomeCount`는 subtype count를 대체하지 않고 그 위에 total cumulative 축을 제공한다.
- repeated refresh나 unrelated event가 total count를 오염시키지 않는 한, current switch audit family의 terminal-outcome-aware envelope를 한 단계 더 직접화하는 변화로 볼 수 있다.

## Decision
- `uTerminalHoldoffOutcomeCount`를 추가한다.
- ordinary complete, bypass complete, abort path에서만 count를 증가시킨다.
- repeated refresh와 unrelated path에서는 증가시키지 않는다.

## Result
- switch audit holdoff outcome wrapper는 이제
  - terminal outcome subtype cumulative count
  - terminal outcome total cumulative count
  를 함께 검증한다.
- current switch audit family는 terminal-outcome-total-aware envelope 기준으로 더 좁혀진다.
