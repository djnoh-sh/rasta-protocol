# Test Specification Draft - Outbound Application Data Send Contract

## Document Control

- Document ID: `TS-013`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Scope

- 테스트 대상:
  - `MOD-001 Public API Layer`
  - `MOD-009 Platform Adapter Layer`
- 관련 요구사항:
  - `FR-003`
  - `IF-001`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-OUT-001 | FR-003, IF-001 | outbound application send 성공 경로 검증 | `ESTABLISHED` 상태 session | `rsrx_session_send_application_data` 호출 | `DATA` frame이 encode되어 transport send 발생 | reason, sequence, confirmation, payload가 설계와 일치 |
| TC-OUT-005 | FR-003, IF-001 | bounded queue guard 검증 | `ESTABLISHED` 상태 session, 첫 outbound send 수행됨 | second/third send 후 fourth send를 호출 | second/third send는 queue에 수용되고, fourth send는 `REJECTED`와 API callback/diagnostic rejection을 발생 | `outstanding 1 + deferred 2`를 넘으면 overflow reject가 관찰 가능하게 남는다 |
| TC-OUT-002 | IF-001 | invalid state guard 검증 | `INITIALIZED` 상태 session | `rsrx_session_send_application_data` 호출 | `INVALID_STATE` 반환 | `ESTABLISHED` 외 상태에서 send를 거부한다 |
| TC-OUT-003 | IF-001 | invalid payload guard 검증 | `ESTABLISHED` 상태 session | null payload + nonzero length로 send 호출 | `INVALID_ARGUMENT` 반환 | 잘못된 payload 조합을 결정적으로 거부한다 |
| TC-OUT-004 | FR-003 | transport adapter direct-send encode 검증 | transport adapter 초기화 완료 | `rsrx_transport_adapter_send_application_data` 호출 | `DATA` frame encode 후 transport send 수행 | reason=`APPLICATION_DATA_REQUESTED`, payload copy, encoded length가 설계와 일치 |
| TC-OUT-006 | FR-003 | transport adapter deferred dispatch 검증 | transport adapter 초기화 완료, 첫 outbound send 수행됨 | second send 시도 후 valid inbound record, 다시 send | second send는 queue에 저장되고, inbound 후 deferred send가 dispatch되며 이후 다시 queue 수용 가능 | adapter outstanding/deferred state가 bounded queue policy와 일치한다 |
| TC-OUT-007 | FR-003 | outbound telemetry 누적 검증 | session/adapter 초기화 완료 | success, repeated busy reject, inbound clear, feedback clear 경로 수행 | accepted/busy/clear telemetry와 consecutive/max busy reject streak, escalation hit telemetry가 기대값으로 누적된다 | direct-send backpressure policy가 관찰 가능한 counter로 남는다 |
| TC-OUT-008 | FR-003 | busy reject threshold escalation 검증 | `uBusyRejectErrorThreshold=2` configured session, first outbound send 수행됨 | busy reject를 2회 연속 발생시킴 | 첫 reject는 warning, 두 번째 reject는 error diagnostic로 승격 | repeated busy reject가 configured threshold에서 escalation policy를 따른다 |
