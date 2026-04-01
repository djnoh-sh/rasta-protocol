# RV-140 Switch Audit Last Completed Cycle Kind Review

## Scope
- last-event 관점에서 마지막 completed holdoff cycle이 ordinary였는지 bypass였는지를 직접 노출한다.

## Observation
- current switch audit는 completed holdoff cycle의 cumulative subtype을
  - ordinary
  - bypass
  로 이미 분리한다.
- 하지만 last-event 관점에서는 `eLastHoldoffCycleState == COMPLETED`만으로는 마지막 completed cycle subtype을 바로 읽을 수 없었다.

## Judgment
- cumulative subtype split과 last-event subtype observability가 같이 있어야 holdoff outcome audit가 더 완전하다.

## Decision
- `eLastCompletedHoldoffCycleKind`를 추가한다.
- ordinary holdoff-complete recovery 시 `ORDINARY`
- bypass completed recovery 시 `BYPASS`
- completed cycle이 아직 없으면 `NONE`
- repeated refresh는 마지막 completed kind를 유지하도록 둔다.

## Result
- holdoff outcome audit는 cumulative bucket뿐 아니라 last completed subtype도 바로 관찰 가능해진다.
- current switch audit family는 holdoff outcome subtype parity 관점에서 representative closeout 상태에 더 가까워진다.
