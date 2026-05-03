# Test Specification Draft - Protocol Codec

## Document Control

- Document ID: `TS-008`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-05-04`

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
| TC-CODEC-006 | FR-004 | buffer too small 검증 | 작은 encode buffer 준비 | encode 수행 | `BUFFER_TOO_SMALL` 반환 | 버퍼 초과 없이 결정적 오류 처리 |
| TC-CODEC-007 | SR-001 | reserved header tamper reject 검증 | reserved header byte가 non-zero인 otherwise well-formed frame 준비 | decode 수행 | `DECODE_ERROR` 반환 | future extension/tamper로 해석될 수 있는 reserved header 오염을 수용하지 않음 |
| TC-CODEC-008 | SR-001, FR-004 | encode null payload with length reject 검증 | payload pointer가 null이고 payload length가 non-zero인 encode request 준비 | encode 수행 | `INVALID_ARGUMENT` 반환 | non-zero payload length에서 null payload dereference가 발생하지 않음 |
