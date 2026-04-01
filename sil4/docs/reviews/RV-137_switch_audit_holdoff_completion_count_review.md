# RV-137 Switch Audit Holdoff Completion Count Review

## Scope
- ordinary holdoff-complete preferred-recovery cumulative count observability를 별도 축으로 고정한다.

## Observation
- current switch audit는 immediate preferred recovery, holdoff-progress umbrella count, bypass preferred recovery를 이미 구분한다.
- 하지만 ordinary holdoff-complete preferred recovery는 broad `uHoldoffPreferredRecoverySwitchCount` 안에 bypass와 함께 묶여 있어, holdoff를 끝까지 채운 recovery와 active-loss bypass recovery를 cumulative count만으로 바로 분리할 수 없었다.

## Judgment
- current holdoff-aware switch audit envelope를 더 명확히 유지하려면 broad holdoff-progress bucket은 retain하되 ordinary holdoff-complete recovery에 대한 전용 cumulative telemetry가 필요하다.

## Decision
- `uHoldoffPreferredRecoverySwitchCount`는 prior holdoff progress가 있었던 preferred recovery 총량으로 유지한다.
- `uCompletedHoldoffPreferredRecoverySwitchCount`를 추가해 ordinary holdoff-complete preferred recovery를 별도 누적한다.
- bypass path는 기존 `uBypassPreferredRecoverySwitchCount`로 계속 분리한다.
- unit/integration에서 ordinary holdoff-complete path와 bypass path를 각각 검증한다.

## Result
- switch audit의 preferred recovery cumulative observability는
  - immediate
  - ordinary holdoff-complete
  - bypass active-loss
  세 bucket으로 분리된다.
- current switch audit family는 holdoff outcome 관점에서도 representative closeout 상태에 더 가까워진다.
