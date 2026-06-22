# RV-168 Switch Audit Terminal Outcome Stability Long-Run Review

## Decision

`terminal outcome` family는 terminal outcome이 이미 기록된 뒤의 repeated active refresh/no-op에서도 cumulative parity와 last-event parity를 유지해야 한다.

## Covered Representative Path

- ordinary holdoff-complete recovery
- post-ordinary active refresh no-op
- abort/reset
- bypass-complete recovery
- post-bypass active refresh no-op

## Rationale

- 기존 mixed long-run path는 ordinary complete, abort, bypass complete 누적 parity를 확인했지만, terminal outcome이 이미 기록된 뒤 이어지는 active no-op이 last terminal outcome과 cumulative count를 오염시키지 않는지까지는 직접 고정하지 않았다.
- current switch audit family는 no-op reason/count telemetry를 이미 갖고 있으므로, terminal outcome family도 terminal outcome 이후 repeated active refresh에 대해 stable retention contract를 가져야 한다.

## Consequence

- `TC-SUP-049`, `TC-INT-167`은 terminal outcome subtype/total count, trigger-origin/event count, last terminal outcome, last completed holdoff cycle kind, last holdoff cycle state가 post-terminal active no-op에서도 유지된다는 representative evidence가 된다.
- `R-003`의 residual은 current terminal outcome family 내부 long-run retention gap보다 broader policy growth와 threshold generalization 쪽으로 더 좁아진다.
