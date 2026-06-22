# RV-154 Switch Audit Holdoff Cycle Start Trigger Review

## Scope

- switch audit holdoff cycle start trigger observability

## Observation

- current switch audit family는 holdoff cycle의 progress, remaining, reset, completion/abort, terminal outcome은 직접 노출한다.
- 하지만 마지막 holdoff cycle이 어떤 transport trigger에서 시작됐는지는 direct telemetry로 읽을 수 없었다.

## Judgment

- current policy 안에서 다음 자연스러운 확장은 terminal outcome보다 앞선 `cycle start trigger`를 direct telemetry로 올리는 것이다.
- 이 값이 있으면 in-progress holdoff, reset 뒤 renewed hold, mixed long-run final cycle의 시작점을 조합 없이 바로 읽을 수 있다.

## Decision

- supervisor report에 아래를 추가한다.
  - `eLastHoldoffCycleStartTriggerEventType`
  - `eLastHoldoffCycleStartTriggerChannelId`
- holdoff cycle first entry 시점에만 갱신하고, recovery/abort 이후에는 last cycle start trigger로 retain한다.

## Result

- current switch audit family는 holdoff progress/outcome뿐 아니라 holdoff cycle의 direct start point까지 representative parity 수준으로 관찰 가능해진다.
