# Low-Level Design Draft - Transport Abstraction

## Document Control

- Document ID: `LLD-006`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-004 Transport Supervisor`
  - `MOD-007 Platform Abstraction`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-003`
  - `FR-004`
  - `IF-002`
  - `SR-002`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_transport.h` | transport abstraction public contract | Public | send/receive/channel query 타입 정의 |
| `tests/unit/test_rsrx_transport_contract.c` | transport header contract smoke test | Internal | 타입 및 인터페이스 계약 확인 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_transport_status_t` | enum | transport layer 결과 코드 | 플랫폼 오류와 상태 전이 코드를 분리 |
| `rsrx_transport_channel_id_t` | enum | 채널 식별자 | primary/secondary/redundant 구분 |
| `rsrx_transport_event_type_t` | enum | transport event 분류 | frame receive, send result, channel up/down |
| `rsrx_transport_send_request_t` | struct | 송신 요청 계약 | payload, length, reason 포함 |
| `rsrx_transport_frame_t` | struct | 수신 frame 계약 | channel, payload, event type 포함 |
| `rsrx_transport_channel_state_t` | struct | 채널 상태 질의 결과 | availability를 명시적으로 표현 |
| `rsrx_transport_port_t` | struct | transport send/receive/query 포트 집합 | 각 callback은 명시적으로 채움 |

## Functional Behavior

- send 포트:
  - 상위 모듈은 payload와 reason code를 함께 전달한다.
  - transport layer는 reason을 telemetry/diagnostic correlation에 사용할 수 있다.
- receive 포트:
  - transport layer는 수신 frame을 구조화된 `rsrx_transport_frame_t`로 제공한다.
  - receive 이벤트는 frame, send result, channel state 변화를 구분해야 한다.
- channel query 포트:
  - orchestrator 또는 supervisor는 명시적으로 채널 availability를 조회할 수 있다.

## Design Rules

- 상위 모듈은 직접 소켓 API를 호출하지 않는다.
- transport payload ownership은 호출자 쪽에 있으며, port contract 자체는 버퍼 수명 정책을 숨기지 않는다.
- redundancy 정책은 channel id와 channel state abstraction 위에서 구현한다.

## Verification Notes

- 필요한 테스트:
  - transport header compile contract 검증
  - send request 구조체 계약 검증
  - receive frame 구조체 계약 검증
  - channel state 구조체 계약 검증
- 분석 포인트:
  - payload pointer nullability 정책
  - channel state와 event type 혼동 금지
  - transport status와 state machine status 혼동 금지
