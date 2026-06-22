# RV-166 Redundancy Stability Envelope Terminal Outcome Inclusion Review

## Decision

`TC-INT-128` redundancy stability envelope closeout integration은 이제 current switch audit terminal outcome envelope을 direct representative path로 포함한다.

## Rationale

- current switch audit family는 `TC-INT-161`과 threshold-aware terminal outcome representative flow들로 ordinary complete, abort, bypass complete cumulative parity까지 closeout 상태다.
- 하지만 broader `redundancy stability envelope` wrapper가 이 family를 직접 포함하지 않으면, current terminal outcome family는 switch audit 내부 closeout에 머물고 broader long-run stability envelope와의 연결은 간접 참조에 가깝다.
- `vTestIntegratedRedundancyStabilityEnvelopeCloseoutFlow()`에 `vTestIntegratedSwitchAuditTerminalOutcomeEnvelopeFlow()`를 포함시키면:
  - hysteresis closeout
  - redundancy long-run closeout
  - flap-bypass closeout
  - threshold closeout
  - switch audit envelope
  - switch audit terminal outcome envelope
  가 하나의 representative stability wrapper 안에서 함께 추적된다.

## Consequence

- `R-003`의 residual은 current terminal outcome family 자체의 representative gap보다 broader long-run stability envelope generalization과 future policy growth 쪽으로 더 좁혀진다.
- current terminal outcome family는 switch audit subfamily closeout일 뿐 아니라 broader redundancy stability envelope에도 direct inclusion 상태로 간주한다.
