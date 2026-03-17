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
| TC-OUT-005 | FR-003, IF-001 | single outstanding send guard 검증 | `ESTABLISHED` 상태 session, 첫 outbound send 수행됨 | 다시 `rsrx_session_send_application_data` 호출 | `REJECTED` 반환, API callback과 diagnostic record가 rejection을 반영 | queue 없이 outstanding send 1건만 허용하고 reject가 관찰 가능하게 남는다 |
| TC-OUT-002 | IF-001 | invalid state guard 검증 | `INITIALIZED` 상태 session | `rsrx_session_send_application_data` 호출 | `INVALID_STATE` 반환 | `ESTABLISHED` 외 상태에서 send를 거부한다 |
| TC-OUT-003 | IF-001 | invalid payload guard 검증 | `ESTABLISHED` 상태 session | null payload + nonzero length로 send 호출 | `INVALID_ARGUMENT` 반환 | 잘못된 payload 조합을 결정적으로 거부한다 |
| TC-OUT-004 | FR-003 | transport adapter direct-send encode 검증 | transport adapter 초기화 완료 | `rsrx_transport_adapter_send_application_data` 호출 | `DATA` frame encode 후 transport send 수행 | reason=`APPLICATION_DATA_REQUESTED`, payload copy, encoded length가 설계와 일치 |
| TC-OUT-006 | FR-003 | transport adapter outstanding clear 검증 | transport adapter 초기화 완료, 첫 outbound send 수행됨 | second send 시도 후 valid inbound record, 다시 send | second send는 `UNAVAILABLE`, inbound 후 send 재허용 | adapter outstanding send state가 direct-send policy와 일치한다 |
| TC-OUT-007 | FR-003 | outbound telemetry 누적 검증 | session/adapter 초기화 완료 | success, busy reject, inbound clear, feedback clear 경로 수행 | accepted/busy/clear telemetry가 기대값으로 누적된다 | direct-send backpressure policy가 관찰 가능한 counter로 남는다 |
