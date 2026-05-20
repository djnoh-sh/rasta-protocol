# Test Specification Draft - Protocol Codec

## Document Control

- Document ID: `TS-008`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-05-20`

## Scope

- 테스트 대상:
  - `MOD-003 Protocol Codec`
- 관련 요구사항:
  - `FR-003`
  - `FR-004`
  - `SR-001`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-CODEC-001 | FR-003 | codec header contract 검증 | header include 가능 | contract test build/run | decode/encode 타입이 정상 사용 가능 | compile 및 실행 성공 |
| TC-CODEC-002 | FR-003, SR-001 | decoded message 구조체 계약 검증 | 없음 | decoded message 인스턴스 생성 | message type, suggested event, sequence, confirm, payload 필드 접근 가능 | 필드 계약이 누락되지 않음 |
| TC-CODEC-003 | FR-004 | encode request/buffer 구조체 계약 검증 | 없음 | encode request 및 buffer 인스턴스 생성 | request와 buffer 필드 접근 가능 | 타입 레이아웃과 값 사용이 결정적 |
| TC-CODEC-004 | FR-003, FR-004 | encode/decode round-trip 검증 | deterministic wire format 사용 | encode 후 decode 수행 | message type, event, reason, sequence, payload가 보존 | round-trip 결과가 설계와 일치 |
| TC-CODEC-005 | SR-001 | unsupported message reject 검증 | invalid message type frame 준비 | decode 수행 | `UNSUPPORTED_MESSAGE` 반환 | 비허용 message 수용 금지 |
| TC-CODEC-006 | FR-004 | buffer too small 검증 | stale `xEncodedLength`를 가진 작은 encode buffer 준비 | encode 수행 | `BUFFER_TOO_SMALL` 반환 및 `xEncodedLength` clear | 버퍼 초과 없이 결정적 오류 처리하고 실패 결과가 stale encoded length를 남기지 않음 |
| TC-CODEC-007 | SR-001 | reserved header tamper reject 검증 | reserved header byte가 non-zero인 otherwise well-formed frame 준비 | decode 수행 | `RESERVED_HEADER_NONZERO` 반환 | future extension/tamper로 해석될 수 있는 reserved header 오염을 generic decode error와 구분해 수용하지 않음 |
| TC-CODEC-008 | SR-001, FR-004 | encode null payload with length reject 검증 | stale `xEncodedLength`를 가진 encode buffer와 payload pointer가 null이고 payload length가 non-zero인 encode request 준비 | encode 수행 | `INVALID_ARGUMENT` 반환 및 `xEncodedLength` clear | non-zero payload length에서 null payload dereference가 발생하지 않고 stale encoded length를 남기지 않음 |
| TC-CODEC-009 | SR-001 | decode trailing bytes reject 검증 | declared payload length는 zero이나 frame에 trailing byte가 붙은 frame 준비 | decode 수행 | `TRAILING_BYTES` 반환 | declared length보다 긴 trailing data를 generic length mismatch와 구분해 수용하지 않음 |
| TC-CODEC-010 | SR-001 | decode truncated payload reject 검증 | declared payload length는 one이나 actual frame에는 payload byte가 없는 frame 준비 | decode 수행 | `TRUNCATED_PAYLOAD` 반환 | declared length보다 짧은 truncated frame을 generic length mismatch와 구분해 수용하지 않음 |
| TC-CODEC-011 | SR-001 | decode oversized declared payload reject 검증 | declared payload length가 `D_RSRX_CODEC_MAX_PAYLOAD_BYTES + 1`이고 actual frame length도 선언값과 일치하는 frame 준비 | decode 수행 | `PAYLOAD_TOO_LARGE` 반환 | bounded decoded payload buffer를 초과하는 declared payload length를 generic decode error와 구분해 수용하지 않음 |
| TC-CODEC-012 | FR-004 | encode oversized payload length reject 검증 | stale `xEncodedLength`를 가진 encode buffer와 payload length가 `D_RSRX_CODEC_MAX_PAYLOAD_BYTES + 1`인 request 준비 | encode 수행 | `PAYLOAD_TOO_LARGE` 반환 및 `xEncodedLength` clear | bounded codec frame capacity를 초과하는 outbound payload를 unsupported-message와 구분해 직렬화하지 않고 stale encoded length를 남기지 않음 |
| TC-CODEC-013 | FR-003, SR-001 | supported message type event mapping 검증 | 모든 supported message type의 zero-payload frame 준비 | decode 수행 | 각 message type이 정의된 suggested event로 mapping됨 | state-machine handoff event가 message type별로 결정적으로 유지됨 |
| TC-CODEC-014 | FR-003, FR-004 | default codec port runtime binding 검증 | `rsrx_codec_get_default_port()`로 기본 port 획득 | port callback으로 encode/decode round-trip 수행 | non-null port와 encode/decode callback이 정상 동작 | adapter/supervisor가 사용할 default codec port binding이 직접 함수와 동일하게 동작 |
| TC-CODEC-015 | FR-004 | encode unsupported message type reject 검증 | stale `xEncodedLength`를 가진 encode buffer와 encode request message type이 `RSRX_MESSAGE_TYPE_INVALID`인 request 준비 | encode 수행 | `UNSUPPORTED_MESSAGE` 반환 및 `xEncodedLength` clear | outbound codec이 unsupported message type을 wire format으로 직렬화하지 않고 stale encoded length를 남기지 않음 |
| TC-CODEC-016 | FR-003, FR-004, SR-001 | codec null argument reject 검증 | stale `xEncodedLength`를 가진 encode buffer와 encode request/buffer/output buffer 또는 decode frame/message/payload pointer가 null인 호출 준비 | encode/decode 수행 | `INVALID_ARGUMENT` 반환, clear 가능한 encode failure는 `xEncodedLength` clear | codec public API가 null pointer 입력을 역참조하지 않고 clear 가능한 encode failure에서 stale encoded length를 남기지 않음 |
| TC-CODEC-017 | SR-001 | decode short header reject 검증 | frame length가 `D_RSRX_CODEC_HEADER_BYTES - 1`인 frame 준비 | decode 수행 | `SHORT_HEADER` 반환 | 최소 header 길이보다 짧은 frame을 generic decode error와 구분해 수용하지 않음 |
| TC-CODEC-018 | FR-003, FR-004 | max payload encode/decode boundary 검증 | payload length가 정확히 `D_RSRX_CODEC_MAX_PAYLOAD_BYTES`인 request 준비 | encode 후 decode 수행 | `OK` 반환, encoded length가 `D_RSRX_CODEC_MAX_FRAME_BYTES`, payload 보존 | 최대 허용 payload 경계값은 정상 frame으로 수용됨 |
| TC-CODEC-019 | SR-001 | reserved header byte matrix reject 검증 | reserved header offsets `2`, `3`, `14`, `15` 중 하나가 non-zero인 otherwise well-formed frame 준비 | decode 수행 | 각 frame이 `RESERVED_HEADER_NONZERO` 반환 | 모든 reserved header 위치가 zero-baseline 위반 시 tamper-specific codec status로 수용되지 않음 |
| TC-CODEC-020 | FR-003, FR-004, SR-001 | unsupported reason code reject 검증 | stale `xEncodedLength`를 가진 encode buffer와 reason byte/code가 defined `rsrx_reason_code_t` 범위 밖인 encode request 및 otherwise well-formed frame 준비 | encode/decode 수행 | encode/decode 모두 `UNSUPPORTED_REASON` 반환, encode failure는 `xEncodedLength` clear | wire/header reason field가 정의되지 않은 상태 이유로 상위 계층에 전달되지 않으며 unsupported message type과 구분되고 stale encoded length를 남기지 않음 |
| TC-CODEC-021 | FR-003, FR-004 | max reason code encode/decode boundary 검증 | reason code가 마지막 defined value인 `RSRX_REASON_INVALID_STATE_VALUE`인 request 준비 | encode 후 decode 수행 | `OK` 반환, reason code 보존 | reason-code 범위 검사가 defined upper boundary를 off-by-one으로 거부하지 않음 |
| TC-CODEC-022 | FR-003, SR-001 | non-frame transport event decode reject 검증 | `eEventType`이 `FRAME_RECEIVED`가 아닌 otherwise well-formed frame 준비 | decode 수행 | `NON_FRAME_EVENT` 반환 | codec decode가 channel/sender lifecycle event를 payload frame으로 오인하지 않으며 direct codec misuse를 generic decode error와 구분함 |
| TC-CODEC-023 | FR-003, SR-001 | invalid transport channel decode reject 검증 | `eChannelId`가 `RSRX_TRANSPORT_CHANNEL_INVALID`인 otherwise well-formed frame 준비 | decode 수행 | `INVALID_CHANNEL` 반환 | codec decode가 invalid transport channel origin을 가진 frame을 generic decode error와 구분해 수용하지 않음 |
| TC-CODEC-024 | FR-003, FR-004, SR-001 | codec wire-profile security-field boundary 검증 | `rsrx_codec_get_wire_profile()` 호출 | profile 조회 | profile id/version과 header/max size가 codec constants와 일치하고 CRC/MAC/timestamp byte size 및 present flag가 모두 `0` | current skeleton PDU가 CRC/MAC/timestamp를 제공하지 않는다는 boundary를 코드와 테스트에서 명시하고 future security parity growth와 구분한다 |
| TC-CODEC-025 | FR-004, SR-001 | CRC32 primitive known-vector and argument guard 검증 | standard CRC32 known vector와 invalid argument 준비 | `rsrx_codec_calculate_crc32()` 호출 | `123456789` CRC가 `0xCBF43926`, null output/null non-empty input은 `INVALID_ARGUMENT`, empty input은 CRC `0` | future CRC wire-format integration 전에 deterministic CRC primitive와 public API guard를 먼저 고정한다 |
| TC-CODEC-026 | FR-003, FR-004, SR-001 | optional CRC32 wire encode/decode 검증 | CRC32 전용 encode/decode API와 payload frame 및 small buffer 준비 | `rsrx_codec_encode_message_with_crc32()` 후 `rsrx_codec_decode_frame_with_crc32()` 수행, small buffer encode 수행 | CRC frame round-trip은 성공하고, CRC append 공간이 부족한 buffer는 `BUFFER_TOO_SMALL` | default skeleton wire path를 바꾸지 않고 optional CRC32 wire path의 append 동작과 bounded-buffer failure mode를 고정한다 |
| TC-CODEC-027 | FR-003, FR-004, SR-001 | CRC32 codec port/profile binding 검증 | `rsrx_codec_get_crc32_port()`와 `rsrx_codec_get_crc32_wire_profile()` 호출 | CRC32 port callback으로 encode/decode round-trip 수행 | CRC32 port는 CRC32 encode/decode 함수에 bind되고 profile은 CRC32 profile id/version, CRC byte size/presence, MAC/timestamp absence, max CRC frame size를 보고한다 | 상위 계층이 default skeleton path와 CRC32 path를 명시적으로 선택할 수 있는 binding contract를 제공한다 |
| TC-CODEC-028 | FR-003, FR-004, SR-001 | CRC32 checksum mismatch status 검증 | CRC32 encode 후 payload byte를 변조한 frame 준비 | `rsrx_codec_decode_frame_with_crc32()` 수행 | CRC mismatch frame은 `RSRX_CODEC_STATUS_CRC_MISMATCH`를 반환한다 | checksum/tamper성 decode failure를 generic decode error와 구분하여 vendor-evidence-friendly negative vector와 richer status taxonomy를 제공한다 |
| TC-CODEC-029 | FR-003, FR-004, SR-001 | CRC32 truncated checksum status 검증 | CRC32 checksum field가 4 bytes보다 짧은 frame 준비 | `rsrx_codec_decode_frame_with_crc32()` 수행 | CRC field가 부족한 frame은 `RSRX_CODEC_STATUS_CRC_TRUNCATED`를 반환한다 | checksum field absence/truncation을 generic malformed-frame decode error와 구분하여 CRC-specific negative vector를 명시한다 |
| TC-CODEC-030 | FR-003, FR-004, SR-001 | codec security capability query 검증 | `rsrx_codec_get_security_capabilities()` 호출 | capability record 조회 | default CRC absent, optional CRC32 available, MAC/timestamp unavailable을 반환한다 | deployment policy가 default skeleton security-field absence와 optional CRC32 선택 가능성을 API level에서 구분해 확인할 수 있다 |
| TC-CODEC-031 | FR-003, FR-004, SR-001 | CRC32 decode null argument reject 검증 | null frame, null decoded-message output, null payload pointer 준비 | `rsrx_codec_decode_frame_with_crc32()` 수행 | 각 null argument path가 `INVALID_ARGUMENT`를 반환한다 | CRC/truncation/mismatch 판정 전에 public API argument guard가 우선 적용된다 |
| TC-CODEC-032 | FR-004, SR-001 | encode failure clears encoded length 검증 | stale `xEncodedLength`를 가진 direct/CRC32 encode buffer와 invalid message request 준비 | `rsrx_codec_encode_message()` 및 `rsrx_codec_encode_message_with_crc32()` 수행 | encode 실패 status를 반환하고 `xEncodedLength`가 `0`으로 clear된다 | 실패한 encode 결과가 이전 성공 길이나 stale 길이를 상위 계층에 남기지 않는다 |
