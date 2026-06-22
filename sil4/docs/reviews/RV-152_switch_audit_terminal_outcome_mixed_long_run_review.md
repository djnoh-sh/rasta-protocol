# RV-152 Switch Audit Terminal Outcome Mixed Long-Run Review

## Scope

- terminal holdoff outcome family의 mixed long-run cumulative parity 검토

## Observation

- ordinary completed path
- aborted reset path
- bypass completed path

는 각각 direct test와 representative wrapper에서 이미 관찰된다.

- 하지만 세 outcome이 한 run 안에서 순차 누적될 때
  - subtype cumulative count
  - total count
  - trigger-origin count
  - trigger-event count
  - last terminal outcome

가 함께 모순 없이 유지되는지는 별도 representative path로 직접 고정돼 있지 않았다.

## Judgment

- current terminal outcome family를 더 좁히려면 새 telemetry 추가보다 mixed long-run representative path가 더 적절하다.
- ordinary complete, abort, bypass complete를 한 session 안에서 연속 재현하면 cumulative coexistence parity를 직접 닫을 수 있다.

## Decision

- unit에 `switch audit terminal outcome mixed long-run matrix`를 추가한다.
- integration에 `switch audit terminal outcome mixed long-run integration`을 추가한다.
- terminal outcome envelope wrapper는 이 mixed long-run representative path까지 포함하도록 확장한다.

## Result

- current terminal outcome family는 single-path parity뿐 아니라 mixed long-run cumulative coexistence parity까지 representative closeout 수준으로 추적 가능해진다.
