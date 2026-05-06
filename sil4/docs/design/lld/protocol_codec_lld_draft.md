# Low-Level Design Draft - Protocol Codec

## Document Control

- Document ID: `LLD-008`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-05-06`

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
| `src/rsrx_codec.c` | deterministic encode/decode skeleton 구현 | Internal | 고정 header 기반 wire format |
| `tests/unit/test_rsrx_codec_contract.c` | codec header contract smoke test | Internal | 타입 계약 및 기본 레이아웃 검증 |
| `tests/unit/test_rsrx_codec.c` | codec round-trip 단위 테스트 | Internal | encode/decode 정상/오류 경로 검증 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_codec_status_t` | enum | codec 결과 코드 | encode/decode 오류를 transport/status와 분리 |
| `rsrx_message_type_t` | enum | protocol message type 분류 | 명시된 message만 허용 |
| `rsrx_decoded_message_t` | struct | frame decode 결과 | suggested event, sequence, confirm, payload 포함 |
| `rsrx_encode_request_t` | struct | message encode 입력 | message type, reason, sequence, payload 포함 |
| `rsrx_encode_buffer_t` | struct | encode 대상 버퍼 | caller-owned buffer 사용 |
| `rsrx_codec_port_t` | struct | encode/decode 함수 집합 | 양 방향 callback 필수 |
| `rsrx_codec_get_default_port` | function | default codec port provider | non-null encode/decode callback 제공 |

## Functional Behavior

- decode:
  - transport frame을 typed decoded message로 변환한다.
  - null frame/message/payload pointer는 `INVALID_ARGUMENT`로 거부한다.
  - frame length가 `D_RSRX_CODEC_HEADER_BYTES`보다 작으면 `DECODE_ERROR`로 거부한다.
  - transport event type이 `RSRX_TRANSPORT_EVENT_FRAME_RECEIVED`가 아니면 `DECODE_ERROR`로 거부한다.
  - transport channel id가 valid channel range 밖이면 `DECODE_ERROR`로 거부한다.
  - decoded message는 state machine에 전달할 `suggested event`를 함께 제공한다.
  - supported message type은 고정된 state-machine suggested event로 mapping한다.
  - reason code가 정의된 `rsrx_reason_code_t` 범위를 벗어나면 `DECODE_ERROR`로 거부한다.
  - 마지막 defined reason code인 `RSRX_REASON_INVALID_STATE_VALUE`는 정상 경계값으로 수용한다.
  - reserved header bytes at offsets `2`, `3`, `14`, and `15`는 zero baseline이어야 하며 non-zero 값은 `DECODE_ERROR`로 거부한다.
  - declared payload length와 actual frame length가 정확히 일치하지 않으면 trailing/short frame 모두 `DECODE_ERROR`로 거부한다.
  - declared payload length가 `D_RSRX_CODEC_MAX_PAYLOAD_BYTES`를 초과하면 frame length가 선언값과 일치하더라도 `DECODE_ERROR`로 거부한다.
  - declared payload length가 정확히 `D_RSRX_CODEC_MAX_PAYLOAD_BYTES`이면 정상 payload 경계값으로 수용한다.
- encode:
  - null request/buffer/output buffer pointer는 `INVALID_ARGUMENT`로 거부한다.
  - message type과 sequence/confirmation/payload를 wire-format buffer로 직렬화한다.
  - unsupported message type은 `UNSUPPORTED_MESSAGE`로 거부한다.
  - unsupported reason code는 `UNSUPPORTED_MESSAGE`로 거부한다.
  - 마지막 defined reason code인 `RSRX_REASON_INVALID_STATE_VALUE`는 정상 경계값으로 직렬화한다.
  - encode 대상 버퍼는 caller가 제공한다.
  - non-zero payload length에서는 payload pointer가 null이면 `INVALID_ARGUMENT`로 거부한다.
  - payload length가 `D_RSRX_CODEC_MAX_PAYLOAD_BYTES`를 초과하면 `UNSUPPORTED_MESSAGE`로 거부한다.
  - payload length가 정확히 `D_RSRX_CODEC_MAX_PAYLOAD_BYTES`이면 정상 payload 경계값으로 수용한다.
  - skeleton 구현은 고정 길이 header와 variable payload로 구성된 deterministic wire format을 사용한다.

## Design Rules

- codec은 transport port나 orchestrator를 직접 호출하지 않는다.
- codec status는 state machine status와 혼용하지 않는다.
- decoded payload는 bounded buffer 안에서만 다룬다.
- default codec port는 직접 codec 함수와 동일한 encode/decode semantics를 제공한다.

## Verification Notes

- 필요한 테스트:
  - codec header compile contract 검증
  - decoded message 구조체 계약 검증
  - encode request/buffer 구조체 계약 검증
  - encode/decode round-trip 검증
  - encode/decode null argument reject 검증
  - short header frame reject 검증
  - non-frame transport event reject 검증
  - invalid transport channel reject 검증
  - unsupported message reject 검증
  - unsupported encode message type reject 검증
  - unsupported reason code reject 검증
  - max reason code encode/decode boundary 검증
  - supported message type별 suggested event mapping 검증
  - default codec port encode/decode binding 검증
  - reserved header tamper reject 검증
  - reserved header byte matrix reject 검증
  - non-zero payload length와 null payload pointer 조합 reject 검증
  - oversized encode payload length reject 검증
  - declared length와 actual frame length mismatch reject 검증
  - oversized declared payload length reject 검증
  - max payload encode/decode boundary 검증
  - buffer too small 검증
- 분석 포인트:
  - payload 최대 길이 상한
  - payload pointer/length consistency
  - declared length와 actual frame length 일치성
  - reserved header bytes zero-baseline 유지
  - message type과 suggested event 매핑 정책
  - sequence/confirmation 필드 overflow 검토
