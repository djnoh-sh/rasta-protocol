# Low-Level Design Draft - Platform Adapter Layer

## Document Control

- Document ID: `LLD-005`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-007 Platform Abstraction`
  - `MOD-008 Connection Orchestrator`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `IF-002`
  - `FR-007`
  - `SR-003`
  - `SR-004`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_platform_adapters.h` | platform-backed executor helper 공개 API | Public | timer/diagnostics executor binding |
| `src/rsrx_platform_adapters.c` | platform-backed executor 구현 | Internal | transport/timer/diagnostics executor table 조립 |
| `tests/unit/test_rsrx_platform_adapters.c` | adapter binding 및 dispatch 단위 테스트 | Internal | platform port stub 사용 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_transport_adapter_context_t` | struct | transport port, codec port, encoded frame buffer, last inbound message cache를 보유 | 동적 메모리 미사용 |
| `rsrx_platform_adapter_context_t` | struct | platform port와 interval 설정 보유 | 동적 메모리 미사용 |
| `rsrx_transport_adapter_init` | function | transport adapter context 초기화 | 유효한 transport/codec port 필요 |
| `rsrx_transport_executor_dispatch` | function | transport action을 encode 후 send request로 변환 | transport action만 처리 |
| `rsrx_transport_adapter_send_application_data` | function | application payload를 직접 `DATA` encode/send로 변환 | null payload + nonzero length 금지 |
| `rsrx_platform_adapter_init` | function | platform adapter context 초기화 | 유효한 port table 필요 |
| `rsrx_platform_timer_executor_dispatch` | function | timer action을 platform timer command로 변환 | timer action만 처리 |
| `rsrx_platform_diagnostics_executor_dispatch` | function | transition result를 diagnostics record로 변환 | bounded 기록 생성 |
| `rsrx_platform_adapter_build_executor_table` | function | transport/timer/application/diagnostics/api/lifecycle executor를 결합 | null 금지 |

## Functional Behavior

- transport adapter:
  - `START_HANDSHAKE`, `ACCEPT_INBOUND_CONNECT`, `SEND_HEARTBEAT`, `REQUEST_RETRANSMISSION`, `SEND_DISCONNECT`를 message type으로 매핑한다.
  - mapped action은 `protocol context`를 통해 sequence/confirmation이 채워진 `codec encode request`로 변환된다.
  - encode 성공 시 encoded wire buffer를 `rsrx_transport_send_request_t`의 payload로 전달한다.
  - inbound decoded message는 protocol context의 confirmation 기준을 갱신하고 마지막 inbound message cache를 보존한다.
  - explicit outbound application send는 `rsrx_transport_adapter_send_application_data`가 담당한다.
  - direct-send helper는 `DATA` frame과 `APPLICATION_DATA_REQUESTED` reason을 사용한다.
- application executor support:
  - `DELIVER_DATA`는 transport adapter가 아니라 별도 application executor가 처리한다.
  - transport adapter는 application executor가 참조할 마지막 inbound message만 제공한다.
- timer adapter:
  - `START_SUPERVISION_TIMER`, `RESET_SUPERVISION_TIMER`를 monotonic deadline 기반 command로 변환한다.
  - 현재 시간은 `rsrx_clock_port_t`를 통해 조회한다.
- diagnostics adapter:
  - transition result를 `rsrx_diagnostic_record_t`로 변환한다.
  - severity는 `diagnostic code`에서 결정한다.
  - event counter는 adapter context 내부에서 증가시킨다.
- executor table builder:
  - transport executor는 transport adapter dispatch 함수로 설정한다.
  - timer/diagnostics executor는 platform adapter dispatch 함수로 설정한다.
  - application/api/lifecycle executor는 외부 제공 executor를 사용한다.

## Verification Notes

- 필요한 테스트:
  - executor table 조립 검증
  - transport action -> encoded send request 변환 검증
  - direct outbound application send 검증
  - inbound message cache 조회 검증
  - timer action -> timer command 변환 검증
  - diagnostics action -> diagnostic record 변환 검증
- 분석 포인트:
  - transport action과 message type mapping의 bounded 정책
  - encoded frame buffer 상한과 codec 실패 시 무송신 정책
  - monotonic deadline 계산 bounded 여부
  - severity mapping 완전성
  - platform callback 실패가 상위 상태 머신 결정성에 영향을 주지 않도록 유지
