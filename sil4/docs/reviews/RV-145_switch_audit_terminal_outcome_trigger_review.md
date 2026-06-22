# RV-145 Switch Audit Terminal Outcome Trigger Review

## Scope
- last terminal holdoff outcome를 만든 trigger를 direct field로 고정한다.

## Observation
- current switch audit는
  - `eLastTerminalHoldoffOutcome`
  - `eLastSwitchTriggerEventType`
  - `eLastSwitchTriggerChannelId`
  를 이미 가진다.
- 하지만 repeated refresh가 지나간 뒤에는 `eLastSwitchTrigger*`가 최신 refresh 이벤트로 갱신되므로, 마지막 terminal holdoff outcome을 만든 원래 trigger는 별도로 보존되지 않았다.

## Judgment
- terminal holdoff outcome taxonomy를 direct field로 올렸다면, 그 outcome을 만든 trigger도 direct field로 보존해야 audit trail이 더 완전하다.

## Decision
- `eLastTerminalHoldoffOutcomeTriggerEventType`
- `eLastTerminalHoldoffOutcomeTriggerChannelId`
  를 추가한다.
- ordinary completed, bypass completed, aborted path에서 각각 terminal outcome trigger를 기록한다.
- repeated refresh에서는 terminal outcome trigger를 overwrite하지 않고 유지한다.

## Result
- holdoff outcome audit는 이제
  - cumulative bucket
  - last completed subtype
  - last terminal outcome
  - last terminal outcome trigger
  를 direct field로 함께 제공한다.
- current switch audit family는 holdoff outcome observability 관점에서 representative closeout 상태에 더 가까워진다.
