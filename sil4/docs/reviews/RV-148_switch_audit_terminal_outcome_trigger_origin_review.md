# RV-148 Switch Audit Terminal Outcome Trigger Origin Review

## Scope
- terminal holdoff outcome의 direct trigger가 preferred channel 쪽인지 non-preferred channel 쪽인지 cumulative count로 분리한다.

## Observation
- current telemetry는 last terminal outcome trigger channel/event를 직접 남기고, terminal outcome subtype 및 total cumulative count도 제공한다.
- 하지만 terminal outcome cumulative history를 trigger origin 기준으로 보는 direct bucket은 아직 없어 ordinary/bypass/abort 흐름의 origin mix를 한 번에 읽기 어렵다.

## Judgment
- holdoff outcome family를 trigger-aware envelope로 더 직접 확장하려면 terminal outcome cumulative count도 preferred-triggered vs non-preferred-triggered 기준으로 나누는 편이 맞다.
- ordinary complete와 abort는 preferred-triggered bucket에, bypass complete는 non-preferred-triggered bucket에 들어간다.

## Decision
- 아래 cumulative field를 추가한다.
  - `uPreferredChannelTriggeredTerminalHoldoffOutcomeCount`
  - `uNonPreferredChannelTriggeredTerminalHoldoffOutcomeCount`
- repeated refresh와 unrelated path는 이 origin count를 오염시키지 않는다.

## Result
- switch audit holdoff outcome wrapper는 이제 terminal outcome에 대해
  - subtype count
  - total count
  - trigger-origin count
  를 함께 검증한다.
- current switch audit family는 terminal-outcome-trigger-origin-aware envelope 기준으로 더 좁혀진다.
