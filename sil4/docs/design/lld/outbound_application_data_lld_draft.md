# Low-Level Design Draft - Outbound Application Data Send Contract

## Document Control

- Document ID: `LLD-013`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-17`

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

- 상위 애플리케이션이 application payload를 protocol `DATA` frame으로 제출하는 명시적 경계를 정의한다.
- 현재 단계에서는 queue 없이 synchronous send API를 사용한다.
- 현재 단계에서는 `single outstanding send only` 정책을 사용한다.
- state machine action 확장 전까지 outbound data는 session API가 직접 transport adapter helper를 호출한다.

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_api.h` | outbound application send API 선언 | Public | 상위 애플리케이션 진입점 |
| `src/rsrx_api.c` | 상태 확인 후 transport adapter로 위임 | Internal | `ESTABLISHED` 상태만 허용 |
| `include/rsrx_platform_adapters.h` | application data send helper 선언 | Public | adapter 경계 |
| `src/rsrx_platform_adapters.c` | payload encode 후 transport send 수행 | Internal | protocol context 사용 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_session_send_application_data` | function | application payload를 outbound data frame으로 제출 | `ESTABLISHED` 상태만 허용 |
| `rsrx_session_get_outbound_telemetry` | function | outbound send accept/reject/clear telemetry 조회 | read-only view |
| `rsrx_transport_adapter_send_application_data` | function | payload를 `DATA` encode/send로 변환 | null payload + nonzero length 금지, outstanding send 존재 시 busy reject |
| `rsrx_transport_adapter_get_outbound_telemetry` | function | adapter outbound telemetry 조회 | read-only view |
| `rsrx_outbound_send_telemetry_t` | struct | last send status, accepted count, busy reject count, consecutive/max busy reject streak, escalation count/latch, clear source count 보유 | cumulative counter는 reset 전까지 유지 |
| `uBusyRejectErrorThreshold` | config field | repeated busy reject가 warning에서 error diagnostic으로 승격되는 임계치 | `0`이면 escalation 비활성화 |
| `RSRX_REASON_APPLICATION_DATA_REQUESTED` | reason code | outbound application data 전송 이유 | transport request와 codec header에 기록 |

## Functional Behavior

- `rsrx_session_send_application_data`:
  - session null, 미초기화 session, invalid payload 조합을 거부한다.
  - 현재 상태가 `ESTABLISHED`가 아니면 `INVALID_STATE`를 반환한다.
  - 허용 상태이면 transport adapter send helper를 호출한다.
  - transport helper가 실패하면 `REJECTED`를 반환한다.
  - direct-send reject가 발생하면 session은 same-state synthetic report를 구성하고 API callback과 diagnostics port에 rejection을 기록한다.
  - `uBusyRejectErrorThreshold > 0` 이고 current busy reject streak가 threshold 이상이면 synthetic diagnostic를 `ERROR_INTERFACE`로 승격한다.
- `rsrx_transport_adapter_send_application_data`:
  - outstanding send가 이미 있으면 `UNAVAILABLE`을 반환한다.
  - `protocol context`를 통해 sequence/confirmation이 채워진 encode request를 생성한다.
  - message type은 `DATA`로 고정한다.
  - reason은 `APPLICATION_DATA_REQUESTED`로 고정한다.
  - encode 성공 시 transport send request를 바로 하위 transport port로 전달한다.
  - valid inbound message가 record되거나 correlated transport feedback이 수신되기 전까지 outstanding send를 유지한다.
- outbound telemetry:
  - successful direct-send마다 `uAcceptedSendCount`를 증가시킨다.
  - outstanding send 존재로 거부되면 `uBusyRejectedSendCount`를 증가시키고 `eLastSendStatus=UNAVAILABLE`을 기록한다.
  - busy reject가 연속되면 `uConsecutiveBusyRejectedSendCount`를 증가시키고, `uMaxConsecutiveBusyRejectedSendCount`를 갱신한다.
  - threshold escalation이 발생하면 `uBusyRejectEscalationCount`를 증가시키고 `uLastBusyRejectEscalated=1`을 기록한다.
  - valid inbound message로 outstanding가 해제되면 `uClearOnInboundCount`를 증가시킨다.
  - correlated transport feedback로 outstanding가 해제되면 `uClearOnFeedbackCount`를 증가시킨다.
  - explicit/manual clear는 `uClearManualCount`를 증가시킨다.
  - successful send 또는 clear가 발생하면 current busy reject streak를 `0`으로 reset하고 escalation latch를 내린다.

## Constraints

- 현재 구현은 synchronous direct-send + single outstanding send 모델이다.
- outbound application data는 API callback이나 lifecycle callback을 발생시키지 않는다.
- busy reject는 queue overflow가 아니라 `single outstanding send only` 정책 위반으로 해석한다.
- busy reject의 synthetic report는 `status=REJECTED`, `reason=APPLICATION_DATA_REQUESTED`, `diagnostic=WARN_REJECTED_EVENT`를 사용한다.
- threshold가 활성화되고 repeated busy reject streak가 임계치 이상이면 synthetic report의 diagnostic는 `ERROR_INTERFACE`를 사용한다.
- queueing, batching, multi-depth backpressure 정책은 후속 단계에서 별도 정의한다.

## Verification Notes

- 필요한 테스트:
  - `ESTABLISHED` 상태 outbound application send 성공
  - outstanding send 존재 시 second send 거부
  - second send reject 시 API callback/diagnostic correlation 검증
  - valid inbound 후 send 재허용
  - accepted/busy reject/clear source telemetry 누적 검증
  - repeated busy reject streak와 reset/max 유지 검증
  - busy reject threshold 도달 시 warning -> error diagnostic 승격 검증
  - escalation count/latch telemetry 갱신 검증
  - `INITIALIZED` 또는 `CONNECTING` 상태 send 거부
  - null payload + nonzero length 거부
  - encoded frame이 `DATA`/`APPLICATION_DATA_REQUESTED`/expected sequence를 포함하는지 검증
- 분석 포인트:
  - direct-send path가 inbound delivery path와 혼동되지 않는지 검토
  - protocol context sequence 증가가 outbound application send에서도 유지되는지 검토
  - outstanding send clear 시점이 transport supervisor correlation 정책과 일치하는지 검토
