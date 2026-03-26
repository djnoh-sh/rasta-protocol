# Test Specification Draft - Channel Manager

## Document Control

- Document ID: `TS-015`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-24`
- Last Updated: `2026-03-25`

## Scope

- 테스트 대상:
  - `MOD-010 Channel Manager`
- 관련 요구사항:
  - `FR-003`
  - `IF-002`
  - `SR-003`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-CHM-001 | FR-003, IF-002 | preferred channel 초기 선택 검증 | primary/secondary available config 준비 | `init`, `select_channel` 호출 | primary 선택, failover 없음 | selected channel, available count, failover flag가 설계와 일치 |
| TC-CHM-002 | FR-003 | active channel unavailable 시 failover 검증 | primary down, secondary up 상태 준비 | `update_channel`, `select_channel` 호출 | secondary 선택, failover 발생 | active channel, failover flag, cumulative switch count가 설계와 일치 |
| TC-CHM-003 | SR-003 | all channel unavailable 처리 검증 | primary/secondary 모두 down 상태 준비 | `select_channel` 호출 | `UNAVAILABLE` 반환 | invalid channel과 unavailable status가 결정적으로 보고됨 |
| TC-CHM-005 | FR-003 | preferred channel recovery auto-switch 검증 | secondary로 failover된 뒤 primary restored 상태 준비 | `update_channel`, `select_channel` 호출 | preferred primary로 자동 복귀 | selected channel, switch flag, cumulative switch count가 preferred recovery policy와 일치 |
| TC-CHM-006 | FR-003 | preferred recovery holdoff 검증 | secondary로 failover된 뒤 primary restored, holdoff `2` config 준비 | `select_channel`을 2회 연속 호출 | 첫 호출은 secondary 유지, 두 번째 호출에서 primary 복귀 | holdoff count 전에는 switch가 억제되고 threshold 도달 후에만 복귀하며 cumulative switch count가 기대값과 일치 |
| TC-CHM-007 | FR-003, SR-003, IF-002 | preferred recovery hysteresis reset matrix 검증 | secondary로 failover된 뒤 primary restore/flap/restore가 반복되는 holdoff `2` config 준비 | `failover -> hold -> primary flap down -> renewed hold -> recovery -> no-op refresh` 순서로 `update_channel`, `select_channel` 호출 | flap down은 holdoff 누적을 reset하고 renewed holdoff 뒤에만 primary 복귀가 허용되며, 복귀 뒤 repeated select는 no-op refresh로 남는다 | selected channel, switch flag, cumulative switch count가 hysteresis reset/no-op refresh policy와 일치 |
| TC-CHM-008 | FR-003, SR-003, IF-002 | active-channel loss가 preferred recovery holdoff를 bypass하는지 검증 | secondary로 failover된 뒤 primary restored, first hold가 누적된 holdoff `2` config 준비 | `failover -> primary restore -> hold -> secondary flap down -> select -> secondary restore -> select` 순서로 `update_channel`, `select_channel` 호출 | holdoff가 누적 중이어도 active secondary가 unavailable이 되면 preferred primary가 즉시 선택되고, 이후 secondary 복구는 no-op refresh로 남는다 | selected channel, switch flag, cumulative switch count가 active-loss bypass/no-op refresh policy와 일치 |
| TC-CHM-009 | FR-003, SR-003, IF-002 | active-loss bypass 이후 다음 cycle에서도 holdoff가 다시 적용되는지 검증 | secondary로 failover된 뒤 primary restore/hold, secondary loss bypass, secondary restore refresh를 한 번 거친 holdoff `2` config 준비 | `first failover -> hold -> secondary loss bypass -> refresh -> second failover -> renewed hold -> renewed recovery` 순서로 `update_channel`, `select_channel` 호출 | active-loss bypass는 즉시 preferred primary 복귀를 허용하지만, 이후 다음 failover cycle에서는 holdoff가 새로 누적되어야만 다시 primary 복귀가 허용된다 | selected channel, switch flag, cumulative switch count가 bypass 이후 holdoff re-entry policy와 일치 |
| TC-CHM-010 | FR-003, SR-003, IF-002 | flap reset과 active-loss bypass를 거친 뒤 다음 cycle에서도 holdoff가 다시 적용되는지 검증 | secondary로 failover된 뒤 primary restore/hold, primary flap reset, renewed hold, secondary loss bypass, secondary restore refresh를 한 번 거친 holdoff `2` config 준비 | `first failover -> hold -> primary flap down -> renewed hold -> secondary loss bypass -> refresh -> second failover -> renewed hold -> renewed recovery` 순서로 `update_channel`, `select_channel` 호출 | flap reset은 holdoff 누적을 초기화하고, active-loss bypass는 즉시 preferred primary 복귀를 허용하지만, 이후 다음 failover cycle에서는 holdoff가 다시 새로 누적되어 첫 recovery signal에서는 secondary를 유지하고 두 번째 recovery signal에서만 primary 복귀가 허용된다 | selected channel, switch flag, cumulative switch count가 flap reset + bypass + holdoff re-entry policy와 일치 |
| TC-CHM-011 | FR-003, SR-003, IF-002 | preferred recovery hysteresis closeout matrix 검증 | holdoff reset, active-loss bypass, bypass 이후 holdoff re-entry, flap-reset 이후 bypass re-entry를 모두 재현 가능한 holdoff `2` config 준비 | hysteresis reset matrix, active-loss bypass matrix, bypass re-entry matrix, flap-reset+bypass re-entry matrix를 closeout wrapper로 실행 | preferred recovery hysteresis 규칙군에서 holdoff reset, bypass, bypass 이후 re-entry semantics가 서로 모순 없이 유지된다 | representative wrapper가 channel manager hysteresis 규칙군의 unit coverage를 하나의 closeout 항목으로 추적 가능하게 유지한다 |
| TC-CHM-012 | FR-003, SR-003, IF-002 | preferred recovery holdoff threshold `3` parity 검증 | secondary로 failover된 뒤 primary restored 상태의 holdoff `3` config 준비 | `failover -> hold -> hold -> recovery` 순서로 `update_channel`, `select_channel` 호출 | 첫 두 번의 stable select는 secondary를 유지하고 세 번째 stable select에서만 primary 복귀가 허용된다 | selected channel, switch flag, cumulative switch count가 higher holdoff threshold policy와 일치 |
| TC-CHM-004 | FR-003 | reset to preferred channel 검증 | failover 후 primary restored 상태 준비 | `reset`, `select_channel` 호출 | preferred primary 재선택 | selected channel이 preferred policy와 일치 |
