# RV-175 Redundancy Feedback Long-Run Wrapper Review

## Scope
- broader redundancy long-run family 안에서 non-terminal feedback branch를 별도 representative integration wrapper로 직접 추적한다.

## Observation
- current long-run branch에는 recovery stale mixed feedback, flap-bypass stale feedback, flap-bypass stale completion 같은 non-terminal feedback path가 이미 있다.
- 하지만 이 경로들은 `redundancy long-run closeout`과 `flap-bypass closeout` 내부에 흩어져 있어, broader long-run family의 feedback branch를 하나의 representative 항목으로 읽기 어렵다.

## Judgment
- longer-run stability residual을 더 직접 좁히려면 새 runtime telemetry 추가보다 non-terminal feedback branch를 별도 wrapper로 올리는 편이 맞다.
- 이렇게 하면 terminal outcome이 없는 long-run branch와 terminal outcome retention branch를 문서와 traceability에서 분리해 해석할 수 있다.

## Decision
- `TC-INT-172` redundancy feedback long-run representative integration을 추가한다.
- wrapper는 아래를 함께 실행한다.
  - `redundancy flap-transient long-run recovery stale mixed feedback`
  - `redundancy recovery stale mixed feedback budget reset long-run`
  - `flap-bypass stale feedback long-run`
  - `flap-bypass stale completion long-run`

## Result
- broader redundancy long-run family는 terminal outcome path와 별개로 non-terminal feedback branch도 representative integration 항목으로 추적 가능하다.
- `R-003` residual은 current feedback long-run branch 부족보다 그 바깥의 threshold family generalization, future switch-audit policy growth, future redundancy mode growth 쪽으로 더 좁아진다.
