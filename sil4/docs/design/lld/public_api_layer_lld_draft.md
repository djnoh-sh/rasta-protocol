# Low-Level Design Draft - Public API Layer

## Document Control

- Document ID: `LLD-007`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-001 Public API Layer`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-001`
  - `FR-003`
  - `FR-005`
  - `IF-001`
  - `SR-004`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_api.h` | public API/session contract 정의 | Public | 상위 애플리케이션 진입점 |
| `src/rsrx_api.c` | session 초기화 및 API call routing 구현 | Internal | orchestrator/adapters 조립 |
| `include/rsrx_config_validator.h` | session config startup gate 계약 | Public | `rsrx_session_init`가 사용 |
| `tests/unit/test_rsrx_api.c` | public API 골격 단위 테스트 | Internal | callback 및 state transition 검증 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_session_config_t` | struct | session 초기화 입력 계약 | transport/codec/platform ports와 callbacks 필수 |
| `rsrx_session_t` | struct | session runtime context | transport/platform adapters와 orchestrator 포함 |
| `rsrx_api_notification_fn` | function pointer | 상태 변화/API notification callback | null 금지 |
| `rsrx_application_data_fn` | function pointer | inbound application data callback | null 금지 |
| `rsrx_lifecycle_notification_fn` | function pointer | lifecycle action callback | null 금지 |
| `rsrx_session_init` | function | session 및 하위 모듈 초기화 | deterministic startup 보장 |
| `rsrx_session_start` | function | `INIT_SUCCESS` 경로 시작 | session을 `INITIALIZED`로 이동 |
| `rsrx_session_connect` | function | 연결 시작 | `CONNECT_REQUEST` 전달 |
| `rsrx_session_disconnect` | function | 연결 종료 요청 | `DISCONNECT_REQUEST` 전달 |
| `rsrx_session_process_event` | function | 일반 event 전달 | orchestrator wrapper |
| `rsrx_session_process_timer_expiry` | function | timer expiry source를 protocol event로 변환 후 전달 | 지원 범위 밖 timer source는 거부 |
| `rsrx_session_send_application_data` | function | application payload를 outbound data frame으로 제출 | `ESTABLISHED` 상태만 허용 |
| `rsrx_session_get_state` | function | session 상태 조회 | 읽기 전용 |
| `rsrx_session_reset` | function | session 상태 초기화 | bounded 동작 |

## Functional Behavior

- `rsrx_session_init`:
  - `rsrx_validate_session_config`를 먼저 호출해 startup gate를 통과한 설정만 허용한다.
  - transport adapter, platform adapter, executor table, orchestrator를 순서대로 초기화한다.
  - application/API/lifecycle callback을 session 내부 executor로 연결한다.
- `rsrx_session_start`:
  - `INIT_SUCCESS` event를 주입해 session을 `INITIALIZED` 상태로 전이한다.
- `rsrx_session_connect`:
  - `CONNECT_REQUEST` event를 주입해 연결 수립 절차를 시작한다.
- `rsrx_session_disconnect`:
  - `DISCONNECT_REQUEST` event를 주입해 safe disconnect 경로를 시작한다.
- `rsrx_session_process_event`:
  - 상위 계층이 decoded event 또는 timer event를 직접 전달할 수 있는 일반 경로를 제공한다.
  - `VALID_DATA`가 수용되면 application executor를 통해 마지막 inbound decoded message를 상위 callback에 전달한다.
- `rsrx_session_process_timer_expiry`:
  - `SUPERVISION`은 `TIMEOUT`으로 변환한다.
  - `RETRANSMISSION`은 `RETRANSMISSION_FAILURE`로 변환한다.
  - 현재 단계에서 `DIAGNOSTIC_FLUSH`는 상태 머신 이벤트로 연결하지 않고 거부한다.
- `rsrx_session_send_application_data`:
  - 현재 단계에서 synchronous direct-send 경로를 제공한다.
  - `ESTABLISHED` 상태만 허용한다.
  - transport adapter를 통해 `DATA` frame encode/send를 수행한다.

## Verification Notes

- 필요한 테스트:
  - session init/start/connect 경로 검증
  - session disconnect 경로 검증
  - timer expiry ingress 검증
  - outbound application data send 검증
  - API notification callback 호출 검증
  - application data callback 호출 검증
  - lifecycle callback 호출 검증
  - invalid argument 방어 검증
- 분석 포인트:
  - config validation 실패 시 partially initialized state가 남지 않는지 검토
  - session 초기화 순서와 partially initialized state 방지
  - callback 호출 시점이 report 내용과 일치하는지 검토
