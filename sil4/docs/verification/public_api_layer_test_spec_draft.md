# Test Specification Draft - Public API Layer

## Document Control

- Document ID: `TS-007`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Scope

- 테스트 대상:
  - `MOD-001 Public API Layer`
- 관련 요구사항:
  - `FR-001`
  - `FR-005`
  - `IF-001`
  - `SR-004`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-API-001 | FR-001, IF-001 | session init/start/connect 경로 검증 | 유효 session config 준비 | `session_init`, `session_start`, `session_connect` 호출 | 상태가 `CONNECTING`까지 순차 전이 | callback, transport, timer 동작이 설계와 일치 |
| TC-API-002 | FR-005, SR-004 | session disconnect 경로 검증 | `ESTABLISHED` 상태 session | `session_disconnect` 호출 | `SAFE_DISCONNECT` 전이와 lifecycle callback 발생 | disconnect reason과 callback 동작이 설계와 일치 |
| TC-API-003 | IF-001 | invalid argument 방어 검증 | null session 또는 미초기화 session | API 호출 | 정의된 invalid argument 처리 | UB 없이 결정적 오류 반환 |
