# Test Specification Draft - Application Data Contract

## Document Control

- Document ID: `TS-012`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-16`

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
| TC-APP-001 | FR-003, IF-001 | inbound data가 application callback으로 전달되는지 검증 | `ESTABLISHED` 상태 session, last inbound message 기록됨 | `session_process_event(VALID_DATA)` 호출 | application callback 1회 호출, payload/reason/sequence/confirmation 전달 | transport send count가 증가하지 않고 indication 값이 기록과 일치 |
| TC-APP-002 | FR-003 | `DELIVER_DATA`가 transport executor로 가지 않는지 검증 | transport adapter 초기화됨 | `DELIVER_DATA` 관련 inbound message 기록 후 조회 | last inbound message 조회 성공 | transport executor가 data send를 수행하지 않는다 |
| TC-APP-003 | IF-001 | application callback 누락을 startup gate에서 차단하는지 검증 | session config 준비 | `pfApplicationData = NULL` 후 validate | `MISSING_REQUIRED_FIELD` 반환 | field가 `APPLICATION_DATA_CALLBACK`으로 식별된다 |

## Notes

- application delivery는 현재 synchronous callback 모델이다.
- callback payload는 adapter 내부 저장본을 참조하므로 비동기 보존 요구는 후속 인터페이스 설계에서 별도로 다룬다.
