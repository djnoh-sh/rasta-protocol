# RV-150 Switch Audit Terminal Outcome Envelope Review

## Scope
- current terminal outcome family를 representative wrapper 단위로 묶어 traceability와 residual 해석을 더 직접화한다.

## Observation
- current terminal outcome family는 이미 아래 축을 direct field와 representative flow로 갖고 있다.
  - taxonomy
  - last trigger
  - subtype cumulative count
  - total cumulative count
  - trigger-origin cumulative count
  - trigger-event cumulative count
  - post-terminal active no-op retention parity
- 다만 문서와 traceability 관점에서는 이 축들이 holdoff outcome wrapper 내부에만 흩어져 있어 terminal outcome family 전체를 하나의 closeout 항목으로 읽기 어렵다.

## Judgment
- 새 low-level semantics를 더 추가하기보다 current terminal outcome family를 envelope wrapper로 묶는 편이 맞다.
- 이렇게 하면 residual이 current family 내부 gap이 아니라 envelope 밖의 future policy growth라는 점을 더 명확히 고정할 수 있다.

## Decision
- 아래 representative wrapper를 추가한다.
  - `TC-SUP-043` terminal outcome envelope matrix
  - `TC-INT-161` terminal outcome envelope integration
  - `TC-SUP-049` terminal outcome stability long-run matrix
  - `TC-INT-167` terminal outcome stability long-run integration
- wrapper는 existing holdoff outcome wrapper를 재사용한다.

## Result
- current switch audit family는 terminal outcome family까지 representative closeout 항목으로 추적 가능해졌다.
- `R-003` residual은 current terminal outcome telemetry나 post-terminal retention parity 부족이 아니라 next policy growth와 longer-run stability envelope 쪽으로 더 좁혀진다.
