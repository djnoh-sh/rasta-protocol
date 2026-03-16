# Low-Level Design Draft - Application Data Contract

## Document Control

- Document ID: `LLD-012`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-16`

## Scope

- 대상 모듈:
  - `MOD-001 Public API Layer`
  - `MOD-009 Platform Adapter Layer`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-003`
  - `IF-001`

## Purpose

- inbound `VALID_DATA`가 outbound transport send로 오해되지 않도록 application delivery 경계를 명시한다.
- `DELIVER_DATA` action은 transport executor가 아니라 application executor가 처리한다.
- session은 마지막으로 수용된 inbound decoded message를 application callback에 그대로 전달한다.

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_api.h` | application data indication 및 callback 계약 정의 | Public | 상위 애플리케이션 전달 경계 |
| `src/rsrx_api.c` | application executor 구현 | Internal | session 내부 callback dispatch |
| `include/rsrx_platform_adapters.h` | last inbound message accessor 정의 | Public | application executor가 사용 |
| `src/rsrx_platform_adapters.c` | inbound message 저장 및 executor table 조립 | Internal | `DELIVER_DATA`를 transport에서 제외 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_application_data_indication_t` | struct | application delivery payload, reason, sequence, confirmation을 전달 | decoded inbound message 기준 |
| `rsrx_application_data_fn` | function pointer | application data callback | null 금지 |
| `pvApplicationDataContext` | config/session field | callback user context | session init 시 저장 |
| `pfApplicationData` | config/session field | application data callback | validator가 필수 여부 검증 |
| `rsrx_transport_adapter_get_last_inbound_message` | function | 마지막 inbound message 조회 | inbound data action 시 사용 |

## Functional Behavior

- session init:
  - `pfApplicationData`와 `pvApplicationDataContext`를 session에 저장한다.
  - application executor를 만들어 executor table에 주입한다.
- inbound frame handoff:
  - transport supervisor는 sequence/confirmation 규칙을 통과한 inbound message만 transport adapter에 기록한다.
  - `VALID_DATA` 또는 recovery 후 수용된 data message는 `xLastInboundMessage`를 갱신한다.
- `DELIVER_DATA` action:
  - orchestrator는 `xApplicationExecutor`로 action을 dispatch한다.
  - application executor는 마지막 inbound message를 읽어 `rsrx_application_data_indication_t`를 구성한다.
  - callback에는 payload pointer, payload length, reason, sequence, confirmation이 전달된다.
- outbound transport:
  - `DELIVER_DATA`는 transport message로 encode되지 않는다.
  - outbound send는 명시적 transport action만 수행한다.

## Constraints

- application callback은 session init 시 필수다.
- last inbound message가 없는 상태에서 `DELIVER_DATA`가 발생하면 callback은 호출되지 않는다.
- application indication payload는 transport adapter 내부 저장본을 참조하므로 callback은 동기 처리 기준으로 사용한다.

## Verification Notes

- 필요한 테스트:
  - config validator가 application callback 누락을 거부하는지 검증
  - executor table이 application executor를 요구하는지 검증
  - inbound data event가 transport send를 증가시키지 않는지 검증
  - application callback이 sequence/confirmation/reason/payload를 전달받는지 검증
- 분석 포인트:
  - transport send path와 application delivery path의 분리 유지
  - inbound message 저장 수명과 callback 사용 시점 일치
