# Low-Level Design Draft - Transport Supervisor

## Document Control

- Document ID: `LLD-009`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-004 Transport Supervisor`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-003`
  - `FR-004`
  - `SR-002`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_transport_supervisor.h` | supervisor public contract 정의 | Public | inbound frame 처리 경계 |
| `src/rsrx_transport_supervisor.c` | frame decode 및 session event handoff 구현 | Internal | codec와 session 연결 |
| `tests/unit/test_rsrx_transport_supervisor.c` | inbound handoff 단위 테스트 | Internal | codec stub 사용 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_supervisor_status_t` | enum | supervisor 결과 코드 | decode/session 오류를 분리 |
| `rsrx_transport_supervisor_report_t` | struct | 마지막 frame, decoded message, session report 보유 | caller는 읽기 전용 사용 |
| `rsrx_transport_supervisor_context_t` | struct | session과 codec port 보유 | 동적 메모리 미사용 |
| `rsrx_transport_supervisor_init` | function | supervisor 초기화 | session, codec decode callback 필수 |
| `rsrx_transport_supervisor_process_frame` | function | frame decode 후 session event 전달 | inbound path 핵심 함수 |

## Functional Behavior

- `rsrx_transport_supervisor_init`:
  - session 포인터와 codec decode port 유효성을 검증한다.
  - 마지막 report를 초기화한다.
- `rsrx_transport_supervisor_process_frame`:
  - transport frame을 codec으로 decode한다.
  - decoded message의 `suggested event`를 session으로 전달한다.
  - 마지막 decoded message와 session report를 저장한다.

## Verification Notes

- 필요한 테스트:
  - inbound handshake frame -> session established 경로 검증
  - invalid argument 방어 검증
  - decode 실패 시 오류 반환 검증
- 분석 포인트:
  - decode 결과와 session event handoff 일관성
  - report 구조체의 마지막 값 보존 정책
