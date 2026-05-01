# Test Specification Draft - Public API Layer

## Document Control

- Document ID: `TS-007`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-05-01`

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
| TC-API-004 | IF-001 | inbound heartbeat handoff 검증 | `ESTABLISHED` 상태 session | `session_process_event(VALID_HEARTBEAT)` 호출 | 상태 유지, supervision timer 재시작, diagnostic 기록 | transport 추가 송신 없이 운영 이벤트가 반영된다 |
| TC-API-005 | FR-003, IF-001 | inbound data handoff 검증 | `ESTABLISHED` 상태 session, inbound message 기록됨 | `session_process_event(VALID_DATA)` 호출 | 상태 유지, application data callback 수행 | 전달 reason, payload, sequence, confirmation과 timer/diagnostic 동작이 설계와 일치 |
| TC-API-006 | FR-004, IF-001, SR-004 | retransmission 경로 검증 | `ESTABLISHED` 상태 session | `SEQUENCE_GAP_DETECTED`, 이어서 `RECOVERY_SUCCESS` 호출 | `RETRANSMISSION_PENDING` 진입 후 `ESTABLISHED` 복귀 | retransmission 요청, API notify, recovery lifecycle가 추적 가능하게 남는다 |
| TC-API-007 | SR-002, IF-001 | supervision timer expiry 검증 | `ESTABLISHED` 상태 session | `session_process_timer_expiry(SUPERVISION)` 호출 | `SAFE_DISCONNECT` 전이와 disconnect action 발생 | timeout reason과 fail-safe action이 설계와 일치 |
| TC-API-008 | FR-004, SR-002, IF-001 | retransmission timer expiry 검증 | `RETRANSMISSION_PENDING` 상태 session | `session_process_timer_expiry(RETRANSMISSION)` 호출 | `SAFE_DISCONNECT` 전이와 disconnect action 발생 | retransmission failure reason과 action이 설계와 일치 |
| TC-API-009 | IF-001 | invalid/unsupported timer source 검증 | invalid timer source 또는 `DIAGNOSTIC_FLUSH` source | `session_process_timer_expiry` 호출 | `INVALID_ARGUMENT` 반환 | 지원 범위 밖 timer source를 결정적으로 거부한다 |
| TC-API-010 | FR-003, IF-001 | outbound application data send 검증 | `ESTABLISHED` 상태 session | `session_send_application_data` 호출 | `DATA` frame이 encode되어 transport로 전달 | reason, sequence, confirmation, payload가 설계와 일치 |
| TC-API-011 | IF-001 | outbound application data state/payload guard 검증 | `INITIALIZED` 또는 `ESTABLISHED` 상태 session | invalid state 또는 invalid payload로 `session_send_application_data` 호출 | `INVALID_STATE` 또는 `INVALID_ARGUMENT` 반환 | direct-send contract의 입력 방어가 결정적이다 |
| TC-API-012 | FR-003, IF-001, SR-003 | session reset channel-manager runtime state 검증 | active-standby session에서 pending flap penalty가 armed된 상태 | `rsrx_session_reset` 호출 후 channel-manager selection 수행 | pending penalty가 clear되고 reset-clear telemetry가 증가한다 | public API reset이 orchestrator뿐 아니라 redundancy runtime state도 deterministic하게 초기화한다 |
| TC-API-013 | FR-003, IF-001 | session reset transport-adapter runtime state 검증 | established session에서 outstanding send와 deferred send가 존재하는 상태 | `rsrx_session_reset` 호출 | outstanding/deferred outbound state와 inbound cache가 clear된다 | public API reset 이후 stale outbound queue/protocol runtime state가 남지 않는다 |
