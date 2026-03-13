# Low-Level Design Draft - Protocol Codec

## Document Control

- Document ID: `LLD-008`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-003 Protocol Codec`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-003`
  - `FR-004`
  - `SR-001`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_codec.h` | codec public contract 정의 | Public | encode/decode 타입 및 port 정의 |
| `tests/unit/test_rsrx_codec_contract.c` | codec header contract smoke test | Internal | 타입 계약 및 기본 레이아웃 검증 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_codec_status_t` | enum | codec 결과 코드 | encode/decode 오류를 transport/status와 분리 |
| `rsrx_message_type_t` | enum | protocol message type 분류 | 명시된 message만 허용 |
| `rsrx_decoded_message_t` | struct | frame decode 결과 | suggested event, sequence, confirm, payload 포함 |
| `rsrx_encode_request_t` | struct | message encode 입력 | message type, reason, sequence, payload 포함 |
| `rsrx_encode_buffer_t` | struct | encode 대상 버퍼 | caller-owned buffer 사용 |
| `rsrx_codec_port_t` | struct | encode/decode 함수 집합 | 양 방향 callback 필수 |

## Functional Behavior

- decode:
  - transport frame을 typed decoded message로 변환한다.
  - decoded message는 state machine에 전달할 `suggested event`를 함께 제공한다.
- encode:
  - message type과 sequence/confirmation/payload를 wire-format buffer로 직렬화한다.
  - encode 대상 버퍼는 caller가 제공한다.

## Design Rules

- codec은 transport port나 orchestrator를 직접 호출하지 않는다.
- codec status는 state machine status와 혼용하지 않는다.
- decoded payload는 bounded buffer 안에서만 다룬다.

## Verification Notes

- 필요한 테스트:
  - codec header compile contract 검증
  - decoded message 구조체 계약 검증
  - encode request/buffer 구조체 계약 검증
- 분석 포인트:
  - payload 최대 길이 상한
  - message type과 suggested event 매핑 정책
  - sequence/confirmation 필드 overflow 검토
