# Low-Level Design Draft - Protocol Context

## Document Control

- Document ID: `LLD-011`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-04-24`

## Scope

- 대상 모듈:
  - `MOD-003 Protocol Context`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-003`
  - `FR-004`
  - `SR-003`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_protocol_context.h` | sequence/confirmation context contract 정의 | Public | outbound encode preparation 책임 |
| `src/rsrx_protocol_context.c` | protocol context 구현 | Internal | bounded counter/state only |
| `tests/unit/test_rsrx_protocol_context.c` | protocol context 단위 테스트 | Internal | sequence/confirmation/retransmission base 검증 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_protocol_context_t` | struct | outbound sequence, inbound sequence, remote confirmation, retransmission state 보유 | 동적 메모리 없음 |
| `rsrx_protocol_context_init` | function | protocol context 초기화 | next tx sequence는 1에서 시작 |
| `rsrx_protocol_context_record_inbound_message` | function | inbound decoded message에서 confirmation 기준 갱신 | 단조 증가 정책 |
| `rsrx_protocol_context_resolve_inbound_event` | function | inbound sequence/confirmation/recovery 규칙 판정 | protocol error와 recovery success를 결정 |
| `rsrx_protocol_context_build_encode_request` | function | outbound message type과 reason을 encode request로 변환 | 결정적 sequence/confirmation 부여 |
| `rsrx_protocol_context_clear_retransmission` | function | retransmission pending context 정리 | recovery success 시 호출 |

## Functional Behavior

- outbound sequence number:
  - outbound encode request는 supported outbound message type에서만 허용되고 `INVALID` 같은 잘못된 type은 `INVALID_ARGUMENT`로 거부한다.
  - 최초 값은 `1`이다.
  - outbound message를 생성할 때마다 증가한다.
  - 다음 outbound sequence가 `UINT32_MAX`에 도달한 상태에서는 wraparound를 만들지 않고 encode request 생성을 `REJECTED`로 거부한다.
  - encode request 실패 경로는 caller에게 stale message, payload, sequence를 남기지 않도록 output request를 `INVALID/NONE/0/null/0` baseline으로 clear한다.
- confirmation number:
  - 마지막으로 기록된 inbound sequence number를 사용한다.
  - record 단계는 protocol context inbound contract 바깥의 `INVALID` message type을 `INVALID_ARGUMENT`로 거부하고 tracking state를 변경하지 않는다.
  - inbound message의 remote confirmation은 마지막으로 관측한 값보다 작아질 수 없다.
  - inbound confirmation은 로컬이 실제로 송신한 마지막 sequence보다 클 수 없다.
  - record 단계에서도 invalid confirmation을 `REJECTED`로 거부하고 sequence/confirmation tracking state를 변경하지 않는다.
  - unsequenced inbound message는 sequence/confirmation tracking state를 갱신하지 않는다.
- inbound sequence validation:
  - sequenced message의 첫 inbound sequence는 `1`이어야 한다.
  - 일반 상태에서 `last_rx + 1`이면 정상 수용, 더 크면 gap, 더 작으면 protocol error다.
  - `last_rx`가 `UINT32_MAX`에 도달한 뒤에는 inbound sequence `0` wraparound를 정상 next sequence로 수용하지 않고 protocol error로 처리한다.
  - retransmission pending 상태에서는 `retransmission_base`와 같은 sequence라도 remote confirmation이 마지막 retransmission request tx sequence 이상일 때만 `RECOVERY_SUCCESS`다.
  - retransmission pending 상태에서 `retransmission_base`보다 작으면 protocol error, 더 크면 gap 유지다.
- retransmission request:
  - 첫 요청 시 `last_rx_sequence + 1`을 base sequence로 저장한다.
  - `last_rx_sequence`가 `UINT32_MAX`인 상태에서 새 retransmission base를 계산해야 하면 sequence `0` wraparound를 만들지 않고 encode request 생성을 `REJECTED`로 거부한다.
  - retransmission request를 송신할 때 해당 tx sequence를 별도로 저장한다.
  - request payload는 4-byte big-endian base sequence를 사용한다.
  - `clear_retransmission` 이후에는 다음 요청에서 base를 다시 계산한다.
- unsequenced inbound message:
  - `CONNECT_REQUEST`, `DISCONNECT`, `DIAGNOSTIC`은 ordering resolve 단계에서 suggested event를 pass-through한다.
  - record 단계에서도 last rx, tx confirmation, remote confirmation, retransmission pending state를 오염시키지 않는다.
  - current baseline decision으로 `CONNECT_REQUEST`는 inbound admission trigger일 뿐 sequenced inbound family에 포함되지 않는다.
  - 따라서 `CONNECT_REQUEST` 이후에도 first sequenced inbound acceptance 기준은 여전히 `CONNECT_RESPONSE`/`HEARTBEAT`/`DATA`/`RETRANSMISSION_REQUEST`의 sequence `1`이다.

## Verification Notes

- 필요한 테스트:
  - outbound sequence progression 검증
  - invalid outbound message type reject 검증
  - outbound sequence wraparound guard 검증
  - inbound sequence wraparound guard 검증
  - retransmission base sequence wraparound guard 검증
  - inbound confirmation tracking 검증
  - retransmission request payload/base sequence 검증
  - inbound confirmation validity 검증
  - invalid confirmation record side-effect 차단 검증
  - invalid inbound message type record reject 검증
  - retransmission pending에서 recovery success 판정 검증
  - unconfirmed recovery frame 거부 검증
  - unsequenced message pass-through와 record side-effect 차단 검증
  - `CONNECT_REQUEST` unsequenced baseline decision 검증
  - invalid argument 검증
- 분석 포인트:
  - counter 증가의 bounded behavior
  - confirmation 추적의 단조성
  - retransmission base의 결정성
  - retransmission pending 중 recovery 판정의 결정성
