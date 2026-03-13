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
  - 정상 연결 수립 경로
  - 잘못된 이벤트 타입 입력
  - timeout 기반 안전 종료
  - retransmission 진입과 복구
  - shutdown / reset 경로
- 필요한 정적분석 포인트:
  - enum 범위 처리
  - 모든 `switch` 분기 완전성
  - 초기화되지 않은 값 사용 금지

## Open Issues

- OI-001: action 표현을 bitmask로 할지 bounded array로 할지 결정 필요
- OI-002: `UNINITIALIZED -> INITIALIZED`를 상태 전이로 볼지, 생성 시점으로 볼지 결정 필요
- OI-003: diagnostics update를 상태 머신 내부에서 할지 외부 orchestrator에서 할지 결정 필요

