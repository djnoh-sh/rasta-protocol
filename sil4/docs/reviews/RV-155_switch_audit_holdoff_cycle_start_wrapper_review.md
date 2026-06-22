# RV-155 Switch Audit Holdoff Cycle Start Wrapper Review

## Scope
- direct holdoff cycle start trigger telemetry를 existing holdoff outcome wrapper에 편입한다.

## Observation
- `RV-154`로 `eLastHoldoffCycleStartTriggerEventType`, `eLastHoldoffCycleStartTriggerChannelId`는 direct field로 올라왔다.
- 하지만 representative closeout 관점에서는 이 필드가 standalone parity처럼 보이고, current holdoff outcome wrapper가 함께 보증하는 field라는 점이 덜 명확했다.

## Judgment
- current policy 안에서는 holdoff cycle start trigger가 independent family가 아니라 holdoff outcome family의 start-point observability다.
- 따라서 residual을 더 정확히 좁히려면 standalone telemetry 추가로 남겨두기보다 existing holdoff outcome wrapper 안으로 편입시키는 편이 맞다.

## Decision
- `TC-SUP-042`, `TC-INT-160` 설명을 갱신해 holdoff cycle start trigger를 holdoff outcome wrapper 보장 범위로 명시한다.
- roadmap의 `R-003` wording도 current holdoff-cycle-start parity가 wrapper 기준 representative closeout 상태라는 점을 반영한다.

## Result
- current switch audit family는 holdoff outcome을
  - cycle start
  - in-progress/reset
  - completed subtype
  - terminal outcome
  까지 wrapper 기준으로 읽을 수 있게 된다.
- `R-003` residual은 current holdoff cycle start observability 부족이 아니라 next policy growth 쪽으로 더 좁아진다.
