# Test Specification Draft - Connection State Machine

## Document Control

- Document ID: `TS-002`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Scope

- 테스트 대상:
  - `MOD-002 Connection State Machine`
- 관련 요구사항:
  - `FR-002`
  - `FR-004`
  - `FR-005`
  - `SR-001`
  - `SR-002`
  - `SR-003`
- 테스트 레벨: `Unit` / `Integration-lite`

## Test Environment

- 빌드 설정:
  - 경고 0건 정책
  - 정적분석 병행
- 실행 환경:
  - host 기반 단위 테스트 환경
- 의존 도구:
  - unit test framework
  - 정적분석 도구

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-SM-001 | FR-002 | 정상 연결 수립 경로 검증 | 초기화 완료 | connect event -> handshake success event | 상태가 `ESTABLISHED`로 전이 | 전이 상태와 action이 설계와 일치 |
| TC-SM-002 | SR-001 | 연결 중 허용되지 않은 이벤트 처리 검증 | `CONNECTING` 상태 | invalid message event | `SAFE_DISCONNECT` 전이 또는 정의된 오류 처리 | 비허용 전이가 발생하지 않음 |
| TC-SM-003 | SR-002 | heartbeat timeout 처리 검증 | `ESTABLISHED` 상태 | timeout event | 안전 종료 관련 action 생성 | timeout 후 연결 유지 금지 |
| TC-SM-004 | FR-004 | retransmission 진입 검증 | `ESTABLISHED` 상태 | sequence gap event | `RETRANSMISSION_PENDING` 전이 | 재전송 action이 생성됨 |
| TC-SM-005 | FR-004 | retransmission 복구 검증 | `RETRANSMISSION_PENDING` 상태 | recovery success event | `ESTABLISHED` 복귀 | 상태와 action이 설계와 일치 |
| TC-SM-006 | FR-005 | 사용자 disconnect 처리 검증 | `ESTABLISHED` 상태 | disconnect request event | `SAFE_DISCONNECT` 또는 종료 경로 | 연결이 정상 상태로 남지 않음 |
| TC-SM-007 | SR-003 | 초기화 전 호출 방어 검증 | 미초기화 컨텍스트 | handle_event 호출 | 정의된 오류 또는 안전한 거부 | UB 없이 결과가 결정적 |
| TC-SM-008 | SR-003 | 잘못된 enum 입력 방어 검증 | 초기화 완료 | invalid event enum | 정의된 오류 처리 | 메모리 손상 없이 종료 |
| TC-SM-009 | FR-002 | `INITIALIZED` 상태에서 허용되지 않은 이벤트 거부 검증 | `INITIALIZED` 상태 | valid_data event | 상태 유지 및 오류 통지 | 비허용 전이 없음 |
| TC-SM-010 | FR-005 | `SAFE_DISCONNECT` 이후 cleanup 완료 경로 검증 | `SAFE_DISCONNECT` 상태 | cleanup_complete event | `INITIALIZED` 복귀 | 자원 정리 action 포함 |
| TC-SM-011 | FR-004 | `RETRANSMISSION_PENDING`에서 invalid response 처리 검증 | `RETRANSMISSION_PENDING` 상태 | invalid_response event | `SAFE_DISCONNECT` 전이 | fail-safe action 생성 |
| TC-SM-012 | SR-003 | `SHUTDOWN` 상태 입력 무시 검증 | `SHUTDOWN` 상태 | any event | `SHUTDOWN` 유지 | action이 `ACT_NONE` 또는 문서화된 무시 동작 |
| TC-SM-013 | FR-002, SR-002 | fail-safe 전이 action 순서 검증 | `CONNECTING` 또는 `ESTABLISHED` 상태 | invalid event 또는 timeout event | action 순서가 설계와 동일 | `SEND_DISCONNECT -> ENTER_FAILSAFE -> NOTIFY_API -> LOG_DIAGNOSTIC` 유지 |
| TC-SM-014 | FR-002 | 정상 연결 요청 action 순서 검증 | `INITIALIZED` 상태 | connect_request event | action 순서가 설계와 동일 | `START_HANDSHAKE -> START_SUPERVISION_TIMER -> NOTIFY_API` 유지 |
| TC-SM-015 | SR-003 | 단일 전이 결과 내 action 중복 금지 검증 | 모든 주요 전이 경로 | 각 전이 결과 action list 확인 | 동일 action 중복 없음 | action uniqueness rule 위반 없음 |

## Transition Coverage Matrix

| State | Must Test Events |
| --- | --- |
| `UNINITIALIZED` | `init_success`, `init_failure`, invalid event |
| `INITIALIZED` | `connect_request`, `valid_inbound_connect`, `shutdown_request`, invalid event |
| `CONNECTING` | `handshake_success`, `timeout`, `invalid_message`, `version_mismatch`, invalid event |
| `ESTABLISHED` | `valid_heartbeat`, `valid_data`, `seq_gap_detected`, `disconnect_request`, `timeout`, `protocol_error` |
| `RETRANSMISSION_PENDING` | `recovery_success`, `valid_heartbeat`, `retransmission_failure`, `invalid_response`, `timeout` |
| `SAFE_DISCONNECT` | `cleanup_complete`, `shutdown_request`, ignored event |
| `SHUTDOWN` | ignored event |

## Fault Injection Cases

| Test ID | Fault | Injection Method | Expected Safe Behavior |
| --- | --- | --- | --- |
| FI-SM-001 | 잘못된 상태값 | 테스트 훅으로 상태 필드 변조 | 오류 반환 또는 `SAFE_DISCONNECT` |
| FI-SM-002 | 잘못된 이벤트값 | 범위 밖 enum 전달 | 결정적 오류 처리 |
| FI-SM-003 | 반복 timeout | timeout event 반복 주입 | 상태가 다시 안전하지 않은 방향으로 변하지 않음 |

## Boundary Cases

- 동일 상태에서 동일 이벤트가 반복될 때 idempotent 정책 확인
- `SAFE_DISCONNECT` 이후 `reset` 없이 새 이벤트가 들어오는 경우 처리 규칙 확인
- `RETRANSMISSION_PENDING` 상태에서 heartbeat와 data event 순서 차이 검증
- `UNINITIALIZED` 상태에서 shutdown 요청 직후 init 관련 이벤트가 뒤늦게 들어오는 경우 검증
- `SHUTDOWN` 상태에서 중복 shutdown 요청이 들어오는 경우 검증
- action array 최대 길이 근접 전이에서 순서와 개수 유지 확인

## Execution Record

- 실행 일자: `TBD`
- 실행자: `TBD`
- 결과 요약: `Draft stage - not executed`
