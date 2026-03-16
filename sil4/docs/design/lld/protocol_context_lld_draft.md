# Low-Level Design Draft - Protocol Context

## Document Control

- Document ID: `LLD-011`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-16`

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
| `rsrx_protocol_context_t` | struct | outbound sequence, last inbound sequence, retransmission state 보유 | 동적 메모리 없음 |
| `rsrx_protocol_context_init` | function | protocol context 초기화 | next tx sequence는 1에서 시작 |
| `rsrx_protocol_context_record_inbound_message` | function | inbound decoded message에서 confirmation 기준 갱신 | 단조 증가 정책 |
| `rsrx_protocol_context_build_encode_request` | function | outbound message type과 reason을 encode request로 변환 | 결정적 sequence/confirmation 부여 |
| `rsrx_protocol_context_clear_retransmission` | function | retransmission pending context 정리 | recovery success 시 호출 |

## Functional Behavior

- outbound sequence number:
  - 최초 값은 `1`이다.
  - outbound message를 생성할 때마다 증가한다.
- confirmation number:
  - 마지막으로 기록된 inbound sequence number를 사용한다.
- retransmission request:
  - 첫 요청 시 `last_rx_sequence + 1`을 base sequence로 저장한다.
  - request payload는 4-byte big-endian base sequence를 사용한다.
  - `clear_retransmission` 이후에는 다음 요청에서 base를 다시 계산한다.

## Verification Notes

- 필요한 테스트:
  - outbound sequence progression 검증
  - inbound confirmation tracking 검증
  - retransmission request payload/base sequence 검증
  - invalid argument 검증
- 분석 포인트:
  - counter 증가의 bounded behavior
  - confirmation 추적의 단조성
  - retransmission base의 결정성
