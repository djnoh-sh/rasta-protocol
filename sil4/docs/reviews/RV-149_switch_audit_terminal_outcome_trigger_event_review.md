# RV-149 Switch Audit Terminal Outcome Trigger Event Review

## Scope
- terminal holdoff outcome을 `CHANNEL_UP` vs `CHANNEL_DOWN` trigger event 기준 cumulative count로 분리한다.

## Observation
- current telemetry는 terminal outcome subtype, total count, trigger origin count, last trigger event/channel을 제공한다.
- 하지만 cumulative history를 event type 기준으로 바로 보는 direct bucket은 없어 recovery-complete와 reset/bypass의 event-type mix를 한 번에 읽기 어렵다.

## Judgment
- terminal outcome family를 trigger-aware envelope로 더 직접 확장하려면 trigger event type 기준 cumulative count를 direct field로 두는 편이 맞다.
- ordinary completed는 `CHANNEL_UP`, bypass completed와 aborted reset은 `CHANNEL_DOWN` bucket에 누적된다.

## Decision
- 아래 cumulative field를 추가한다.
  - `uChannelUpTriggeredTerminalHoldoffOutcomeCount`
  - `uChannelDownTriggeredTerminalHoldoffOutcomeCount`
- repeated refresh와 unrelated path는 이 count를 오염시키지 않는다.

## Result
- switch audit holdoff outcome wrapper는 이제 terminal outcome에 대해
  - subtype count
  - total count
  - trigger-origin count
  - trigger-event count
  를 함께 검증한다.
- current switch audit family는 terminal-outcome-trigger-event-aware envelope 기준으로 더 좁혀진다.
