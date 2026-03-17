# Low-Level Design Draft - Transport Supervisor

## Document Control

- Document ID: `LLD-009`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-17`

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
| `rsrx_transport_supervisor_report_t` | struct | 마지막 frame, decoded message, effective event, decision, session report, channel switch telemetry 보유 | caller는 읽기 전용 사용 |
| `rsrx_transport_supervisor_context_t` | struct | session과 codec port 보유 | 동적 메모리 미사용 |
| `rsrx_transport_supervisor_init` | function | supervisor 초기화 | session, codec decode callback 필수 |
| `rsrx_transport_supervisor_process_frame` | function | frame decode 후 session event 전달 | inbound path 핵심 함수 |
| `rsrx_transport_supervisor_poll_receive` | function | channel query 후 frame 수신 polling 수행 | runtime loop 진입점 |
| `rsrx_transport_supervisor_pump_receive` | function | bounded polling loop를 supervisor 내부에서 수행 | integration-facing drain entry |
| `rsrx_transport_supervisor_process_transport_event` | function | send/channel transport event 처리 | outbound/runtime feedback 경계 |
| `rsrx_transport_supervisor_process_timer_expiry` | function | timer expiry를 session으로 위임 | supervisor-level timer ingress |

## Functional Behavior

- `rsrx_transport_supervisor_init`:
  - session 포인터와 codec decode port 유효성을 검증한다.
  - 마지막 report를 초기화한다.
- `rsrx_transport_supervisor_process_frame`:
  - transport frame을 codec으로 decode한다.
  - decoded message를 protocol context 규칙으로 평가해 effective event를 결정한다.
  - sequence가 기대값보다 크면 `SEQUENCE_GAP_DETECTED`로 변환한다.
  - stale/duplicate sequence는 `PROTOCOL_ERROR`로 변환한다.
  - in-order frame만 protocol context에 기록한다.
  - 마지막 decoded message, effective event, session status, supervisor decision, session report를 저장한다.
- `rsrx_transport_supervisor_poll_receive`:
  - transport adapter를 통해 active channel 상태를 조회한다.
  - channel이 unavailable이면 `CHANNEL_DOWN`을 반환하고 receive는 수행하지 않는다.
  - channel이 available이면 frame 수신을 시도한다.
  - 수신 결과가 `UNAVAILABLE`이면 `NO_FRAME`을 반환한다.
  - `FRAME_RECEIVED` event인 경우에만 `process_frame` 경로로 위임한다.
  - poll count, 마지막 channel state, channel manager의 누적 switch count를 report에 남긴다.
- `rsrx_transport_supervisor_pump_receive`:
  - 최대 `uMaxPolls`만큼 `poll_receive`를 반복한다.
  - `OK`는 계속 진행한다.
  - `NO_FRAME`는 종료 조건으로 사용한다.
  - 이미 하나 이상의 frame을 처리한 뒤 `NO_FRAME`를 만나면 전체 결과는 `OK`로 정규화한다.
  - `CHANNEL_DOWN`, `RECEIVE_ERROR`, `DECODE_FAILED`, `SESSION_ERROR`는 즉시 반환한다.
  - 마지막 pump iteration 수와 이번 pump에서 처리한 frame 수를 report에 남긴다.
- `rsrx_transport_supervisor_process_transport_event`:
  - supervisor는 consecutive send failure budget을 내부적으로 유지한다.
  - `SEND_COMPLETED`와 정상 inbound frame 처리는 send failure budget을 reset한다.
  - `SEND_FAILED`는 budget 임계치 미만에서는 ignored event로 기록하고, 임계치 도달 시 `PROTOCOL_ERROR`를 session에 전달한다.
  - `CHANNEL_DOWN`은 transport adapter를 통해 현재 active channel을 다시 조회한다.
  - down된 channel 외에 대체 channel이 available이면 failover를 사용하고 event를 ignored로 처리한다.
  - 대체 channel이 없을 때만 conservative mapping으로 `PROTOCOL_ERROR`를 session에 전달한다.
  - `FRAME_RECEIVED`는 direct frame path로 위임한다.
  - 각 경로는 report에 마지막 decision, current channel switch count, 이번 처리에서 switch가 발생했는지 여부를 남긴다.
- `rsrx_transport_supervisor_process_timer_expiry`:
  - supervisor는 timer source를 해석하지 않고 session timer API로 위임한다.
  - session이 `REJECTED`를 반환해도 supervisor 관점에서는 처리된 fail-safe 전이로 간주한다.
  - 위임 결과는 report에 session status와 decision으로 남긴다.

## Verification Notes

- 필요한 테스트:
  - inbound handshake frame -> session established 경로 검증
  - invalid argument 방어 검증
  - decode 실패 시 오류 반환 검증
  - sequence gap -> retransmission pending 검증
  - stale sequence -> protocol error fail-safe 검증
  - poll receive handshake 경로 검증
  - poll receive channel down/no-frame 경로 검증
  - bounded pump receive drain 검증
  - send failed -> protocol error 검증
  - send completed ignored 검증
  - channel down failover ignored 검증
  - send failure budget reset 검증
  - timer expiry delegation 검증
- 분석 포인트:
  - decode 결과와 supervisor-level event override 일관성
  - protocol context 기록 시점과 sequence rule의 결정성
  - report 구조체의 마지막 값 보존 정책
  - report의 effective event / decision / session status 일관성
  - report의 channel switch telemetry와 channel manager state 일관성
  - query/receive 순서와 channel availability gate의 결정성
  - transport feedback event의 보수적 매핑 정책
  - transient send failure와 persistent send failure 구분 정책
