# Low-Level Design Draft - Connection State Machine

## Document Control

- Document ID: `LLD-002`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-002 Connection State Machine`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-002`
  - `FR-004`
  - `FR-005`
  - `SR-001`
  - `SR-002`
  - `SR-003`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/sil4_state_machine.h` | 상태 머신 public API 정의 | Public | 상위 API layer와 transport supervisor가 참조 |
| `src/sil4_state_machine.c` | 상태 전이와 action 생성 구현 | Internal | 인증 대상 핵심 로직 |
| `tests/unit/test_state_machine.c` | 상태 전이 단위 테스트 | Internal | 모든 허용/비허용 이벤트 조합 검증 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `sil4_state_t` | enum | 상태 정의 | 명시된 상태 외 값 금지 |
| `sil4_event_t` | enum | 상태 전이를 유발하는 이벤트 정의 | 이벤트는 타입과 원인을 분리해 표현 |
| `sil4_action_t` | enum | 상태 전이 후 수행할 후속 action 정의 | side effect는 별도 계층에서 수행 |
| `sil4_transition_result_t` | struct | 새 상태, action 목록, 오류 코드 포함 | bounded action count 유지 |
| `state_machine_init` | function | 컨텍스트 초기화 | 초기 상태는 `UNINITIALIZED` 또는 `INITIALIZED` 중 설계 선택 필요 |
| `state_machine_handle_event` | function | 현재 상태와 이벤트를 받아 전이 결과 생성 | side effect 금지 |
| `state_machine_get_state` | function | 현재 상태 조회 | 읽기 전용 |
| `state_machine_reset` | function | 정리 후 초기 상태 복귀 | shutdown 후에만 허용 |

## State, Event, Action Catalog

### State Catalog

| State | Description | Entry Condition | Exit Condition |
| --- | --- | --- | --- |
| `UNINITIALIZED` | 컨텍스트가 아직 사용 준비되지 않은 상태 | 전원 인가 직후 또는 reset 직후 | `init_success`, `init_failure` |
| `INITIALIZED` | 설정 검증과 초기화는 끝났지만 연결은 시작되지 않은 상태 | 초기화 성공 | `connect_request`, `valid_inbound_connect`, `shutdown_request` |
| `CONNECTING` | 핸드셰이크 진행 중 상태 | 연결 요청 시작 또는 inbound connect 수락 | `handshake_success`, `invalid_message`, `version_mismatch`, `timeout`, `shutdown_request` |
| `ESTABLISHED` | 정상 데이터/heartbeat 교환 상태 | 핸드셰이크 성공 | `seq_gap_detected`, `protocol_error`, `timeout`, `disconnect_request`, `shutdown_request` |
| `RETRANSMISSION_PENDING` | 재전송 복구가 필요한 상태 | sequence gap 검출 | `recovery_success`, `retransmission_failure`, `timeout`, `invalid_response`, `shutdown_request` |
| `SAFE_DISCONNECT` | 안전 종료 처리 진행 상태 | 오류 또는 사용자 종료 요청 | `cleanup_complete`, `shutdown_request` |
| `SHUTDOWN` | 시스템 종료 상태 | 초기화 실패 또는 명시적 종료 | 없음 |

### Event Catalog

| Event | Description | Typical Source |
| --- | --- | --- |
| `init_success` | 초기화 및 설정 검증 성공 | Public API Layer |
| `init_failure` | 초기화 실패 | Public API Layer / Configuration Validator |
| `connect_request` | outbound 연결 요청 | Public API Layer |
| `valid_inbound_connect` | inbound 연결 요청 수용 가능 | Transport Supervisor / Protocol Decoder |
| `handshake_success` | 핸드셰이크 완료 | Protocol handling layer |
| `valid_heartbeat` | 정상 heartbeat 수신 | Transport Supervisor |
| `valid_data` | 정상 데이터 수신 | Transport Supervisor |
| `seq_gap_detected` | sequence gap 검출 | Protocol handling layer |
| `recovery_success` | retransmission 복구 완료 | Protocol handling layer |
| `disconnect_request` | 사용자 또는 peer에 의한 종료 요청 | Public API Layer / Protocol handling layer |
| `timeout` | 감독 시간 초과 | Timer abstraction |
| `invalid_message` | 허용되지 않은 메시지 | Protocol Decoder |
| `version_mismatch` | 버전 불일치 | Protocol handling layer |
| `protocol_error` | 일반 프로토콜 오류 | Protocol handling layer |
| `retransmission_failure` | retransmission 실패 | Protocol handling layer |
| `invalid_response` | 재전송/핸드셰이크 응답 오류 | Protocol handling layer |
| `shutdown_request` | 시스템 종료 요청 | Public API Layer |
| `cleanup_complete` | 안전 종료 후 정리 완료 | Orchestrator |

### Action Catalog

| Action | Description | Executed By |
| --- | --- | --- |
| `ACT_NONE` | 추가 동작 없음 | Caller |
| `ACT_START_HANDSHAKE` | 연결 설정용 송신 동작 시작 | API / Transport layer |
| `ACT_ACCEPT_INBOUND_CONNECT` | inbound 연결 수락 준비 | Orchestrator |
| `ACT_START_SUPERVISION_TIMER` | timeout 감독 시작 | Timer abstraction |
| `ACT_RESET_SUPERVISION_TIMER` | timeout 감독 갱신 | Timer abstraction |
| `ACT_SEND_HEARTBEAT` | heartbeat 전송 | Transport supervisor |
| `ACT_DELIVER_DATA` | 상위 계층에 데이터 전달 허용 | Public API / Adapter layer |
| `ACT_REQUEST_RETRANSMISSION` | retransmission 요청 송신 | Transport supervisor |
| `ACT_CLEAR_RETRANSMISSION_CONTEXT` | 재전송 관련 내부 상태 정리 | Orchestrator |
| `ACT_SEND_DISCONNECT` | disconnect 메시지 전송 | Transport supervisor |
| `ACT_ENTER_FAILSAFE` | fail-safe 종료 처리 시작 | Orchestrator |
| `ACT_LOG_DIAGNOSTIC` | 진단/오류 로그 기록 | Diagnostics layer |
| `ACT_RELEASE_CONNECTION_RESOURCES` | connection 관련 자원 정리 | Orchestrator |
| `ACT_NOTIFY_API` | 상태 변화 또는 오류 통지 | Public API Layer |
| `ACT_FINALIZE_SHUTDOWN` | 최종 종료 처리 | Orchestrator |

## Explicit Transition Table

| Current State | Event | Next State | Actions | Error Policy |
| --- | --- | --- | --- | --- |
| `UNINITIALIZED` | `init_success` | `INITIALIZED` | `ACT_LOG_DIAGNOSTIC`, `ACT_NOTIFY_API` | 없음 |
| `UNINITIALIZED` | `init_failure` | `SHUTDOWN` | `ACT_LOG_DIAGNOSTIC`, `ACT_FINALIZE_SHUTDOWN`, `ACT_NOTIFY_API` | 즉시 시작 실패 |
| `UNINITIALIZED` | `shutdown_request` | `SHUTDOWN` | `ACT_FINALIZE_SHUTDOWN` | 즉시 종료 |
| `UNINITIALIZED` | any other event | `UNINITIALIZED` | `ACT_LOG_DIAGNOSTIC`, `ACT_NOTIFY_API` | 호출 거부 |
| `INITIALIZED` | `connect_request` | `CONNECTING` | `ACT_START_HANDSHAKE`, `ACT_START_SUPERVISION_TIMER`, `ACT_NOTIFY_API` | 없음 |
| `INITIALIZED` | `valid_inbound_connect` | `CONNECTING` | `ACT_ACCEPT_INBOUND_CONNECT`, `ACT_START_SUPERVISION_TIMER`, `ACT_NOTIFY_API` | 없음 |
| `INITIALIZED` | `shutdown_request` | `SHUTDOWN` | `ACT_FINALIZE_SHUTDOWN`, `ACT_NOTIFY_API` | 정상 종료 |
| `INITIALIZED` | any other event | `INITIALIZED` | `ACT_LOG_DIAGNOSTIC`, `ACT_NOTIFY_API` | 호출 거부 |
| `CONNECTING` | `handshake_success` | `ESTABLISHED` | `ACT_RESET_SUPERVISION_TIMER`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 없음 |
| `CONNECTING` | `valid_heartbeat` | `CONNECTING` | `ACT_RESET_SUPERVISION_TIMER`, `ACT_LOG_DIAGNOSTIC` | heartbeat만으로 성공 처리하지 않음 unless handshake policy says so |
| `CONNECTING` | `timeout` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | timeout fail-safe |
| `CONNECTING` | `invalid_message` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | protocol violation |
| `CONNECTING` | `version_mismatch` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | compatibility reject |
| `CONNECTING` | `shutdown_request` | `SHUTDOWN` | `ACT_SEND_DISCONNECT`, `ACT_RELEASE_CONNECTION_RESOURCES`, `ACT_FINALIZE_SHUTDOWN`, `ACT_NOTIFY_API` | 정상 종료 |
| `CONNECTING` | any other event | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 보수적 종료 |
| `ESTABLISHED` | `valid_heartbeat` | `ESTABLISHED` | `ACT_RESET_SUPERVISION_TIMER`, `ACT_LOG_DIAGNOSTIC` | 없음 |
| `ESTABLISHED` | `valid_data` | `ESTABLISHED` | `ACT_RESET_SUPERVISION_TIMER`, `ACT_DELIVER_DATA`, `ACT_LOG_DIAGNOSTIC` | 없음 |
| `ESTABLISHED` | `seq_gap_detected` | `RETRANSMISSION_PENDING` | `ACT_REQUEST_RETRANSMISSION`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | retransmission 시작 |
| `ESTABLISHED` | `disconnect_request` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API` | 정상 종료 또는 peer 종료 |
| `ESTABLISHED` | `timeout` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | timeout fail-safe |
| `ESTABLISHED` | `protocol_error` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | protocol violation |
| `ESTABLISHED` | `shutdown_request` | `SHUTDOWN` | `ACT_SEND_DISCONNECT`, `ACT_RELEASE_CONNECTION_RESOURCES`, `ACT_FINALIZE_SHUTDOWN`, `ACT_NOTIFY_API` | 정상 종료 |
| `ESTABLISHED` | any other event | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 보수적 종료 |
| `RETRANSMISSION_PENDING` | `recovery_success` | `ESTABLISHED` | `ACT_CLEAR_RETRANSMISSION_CONTEXT`, `ACT_RESET_SUPERVISION_TIMER`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 복구 완료 |
| `RETRANSMISSION_PENDING` | `valid_heartbeat` | `RETRANSMISSION_PENDING` | `ACT_RESET_SUPERVISION_TIMER`, `ACT_LOG_DIAGNOSTIC` | 복구 완료 전 상태 유지 |
| `RETRANSMISSION_PENDING` | `retransmission_failure` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 복구 실패 |
| `RETRANSMISSION_PENDING` | `invalid_response` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 응답 오류 |
| `RETRANSMISSION_PENDING` | `timeout` | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | timeout fail-safe |
| `RETRANSMISSION_PENDING` | `shutdown_request` | `SHUTDOWN` | `ACT_SEND_DISCONNECT`, `ACT_RELEASE_CONNECTION_RESOURCES`, `ACT_FINALIZE_SHUTDOWN`, `ACT_NOTIFY_API` | 정상 종료 |
| `RETRANSMISSION_PENDING` | any other event | `SAFE_DISCONNECT` | `ACT_SEND_DISCONNECT`, `ACT_ENTER_FAILSAFE`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 보수적 종료 |
| `SAFE_DISCONNECT` | `cleanup_complete` | `INITIALIZED` | `ACT_RELEASE_CONNECTION_RESOURCES`, `ACT_NOTIFY_API`, `ACT_LOG_DIAGNOSTIC` | 재시작 가능 상태 복귀 |
| `SAFE_DISCONNECT` | `shutdown_request` | `SHUTDOWN` | `ACT_RELEASE_CONNECTION_RESOURCES`, `ACT_FINALIZE_SHUTDOWN`, `ACT_NOTIFY_API` | 종료 |
| `SAFE_DISCONNECT` | any other event | `SAFE_DISCONNECT` | `ACT_LOG_DIAGNOSTIC` | 추가 입력 무시 |
| `SHUTDOWN` | any event | `SHUTDOWN` | `ACT_NONE` | 입력 무시 |

## Function Design

### Function

- Name: `state_machine_init`
- Related Req IDs: `FR-001`, `SR-003`
- Preconditions:
  - 전달된 컨텍스트 포인터가 유효해야 한다.
- Postconditions:
  - 컨텍스트는 문서화된 초기 상태를 가진다.
  - 내부 오류 코드와 action buffer는 초기화된다.
- Error Handling:
  - null 입력 시 정의된 오류 코드 반환
- Side Effects:
  - 없음
- Timing Constraints:
  - 상수 시간 내 완료되어야 한다.

### Function

- Name: `state_machine_handle_event`
- Related Req IDs: `FR-002`, `FR-004`, `FR-005`, `SR-001`, `SR-002`
- Preconditions:
  - 상태 머신이 초기화되어 있어야 한다.
  - 이벤트 값은 유효 enum 범위여야 한다.
- Postconditions:
  - 전이 결과 구조체에 새 상태와 action 목록이 채워진다.
  - 허용되지 않은 이벤트는 안전한 오류 전이로 귀결된다.
- Error Handling:
  - 잘못된 이벤트나 컨텍스트는 안전 종료 action 또는 오류 코드로 변환
- Side Effects:
  - 없음. 실제 송신/로그/타이머 변경은 호출자 계층이 수행
- Timing Constraints:
  - 이벤트 처리 시간은 입력 이벤트 종류와 무관하게 bounded 되어야 한다.

### Function

- Name: `state_machine_get_state`
- Related Req IDs: `IF-001`
- Preconditions:
  - 컨텍스트가 초기화되어 있어야 한다.
- Postconditions:
  - 현재 상태를 반환한다.
- Error Handling:
  - null 입력 시 오류 상태 반환 정책 필요
- Side Effects:
  - 없음
- Timing Constraints:
  - 상수 시간

### Function

- Name: `state_machine_reset`
- Related Req IDs: `FR-005`, `SR-003`
- Preconditions:
  - 컨텍스트가 유효해야 한다.
- Postconditions:
  - 내부 상태와 오류 정보를 초기화한다.
- Error Handling:
  - 잘못된 포인터 입력 시 오류 반환
- Side Effects:
  - 없음
- Timing Constraints:
  - bounded

## Algorithms

- 주요 로직 설명:
  - 상태 머신은 `(현재 상태, 이벤트)`를 키로 하는 명시적 전이 테이블 기반으로 구현한다.
  - 전이 결과는 `next_state`, `action_mask` 또는 bounded action array, `error_code`를 포함한다.
  - 허용되지 않은 전이는 기본적으로 `SAFE_DISCONNECT` 또는 명시된 오류 반환으로 처리한다.
- 경계 조건:
  - 반복 timeout 이벤트
  - 이미 종료된 상태에서 추가 disconnect 이벤트
  - 연결 중 수신되는 out-of-order 이벤트
  - retransmission 중 heartbeat 및 data 경쟁 입력
- 예외 조건:
  - 잘못된 이벤트 enum
  - 초기화 전 호출
  - shutdown 이후 재사용 정책 불명확

## State and Data Ownership

- 소유 데이터:
  - 현재 상태
  - 마지막 오류 코드
  - 최근 이벤트 메타데이터
  - 진단 카운터 일부 참조값
- 초기화 규칙:
  - 모든 필드는 `state_machine_init`에서 명시적으로 초기화한다.
- 해제 규칙:
  - 상태 머신은 동적 메모리를 소유하지 않는 방향을 기본 원칙으로 한다.

## Verification Notes

- 필요한 테스트:
  - 상태별 허용 이벤트 전이 검증
  - 상태별 비허용 이벤트 fail-safe 검증
  - timeout 기반 안전 종료
  - retransmission 진입과 복구
  - shutdown / reset 경로
  - action 목록 생성 정확성 검증
- 필요한 정적분석 포인트:
  - enum 범위 처리
  - 모든 `switch` 분기 완전성
  - 초기화되지 않은 값 사용 금지

## Open Issues

- OI-001: action 표현을 bitmask로 할지 bounded array로 할지 결정 필요
- OI-002: `CONNECTING` 상태에서 `valid_heartbeat`를 handshake success와 분리할지 여부를 상위 핸드셰이크 설계와 일치시켜야 함
- OI-003: diagnostics update를 상태 머신 내부에서 할지 외부 orchestrator에서 할지 결정 필요
