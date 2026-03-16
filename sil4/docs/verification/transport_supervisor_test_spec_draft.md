# Test Specification Draft - Transport Supervisor

## Document Control

- Document ID: `TS-009`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Scope

- 테스트 대상:
  - `MOD-004 Transport Supervisor`
- 관련 요구사항:
  - `FR-003`
  - `FR-004`
  - `SR-002`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-SUP-001 | FR-003, FR-004 | inbound frame handoff 검증 | `CONNECTING` 상태 session, codec decode stub 준비 | `FRAME_RECEIVED` frame 처리 | decoded event가 session에 전달되어 `ESTABLISHED` 전이 | decoded message와 session report가 설계와 일치 |
| TC-SUP-002 | SR-002 | invalid argument 방어 검증 | null context 또는 invalid frame | supervisor init/process 호출 | `INVALID_ARGUMENT` 반환 | UB 없이 결정적 오류 처리 |
| TC-SUP-003 | SR-002 | decode failure 방어 검증 | `CONNECTING` 상태 session, codec이 `DECODE_ERROR` 반환하도록 준비 | `FRAME_RECEIVED` frame 처리 | `DECODE_FAILED` 반환, session 미호출, processed count 미증가 | session state가 유지되고 supervisor report가 마지막 frame만 보존 |
| TC-SUP-004 | FR-003, SR-002 | unsupported message 방어 검증 | `CONNECTING` 상태 session, codec이 `UNSUPPORTED_MESSAGE` 반환하도록 준비 | 지원하지 않는 message frame 처리 | `DECODE_FAILED` 반환, decoded type은 기록되지만 session 미호출 | unsupported message가 보고서에 남고 상태 전이는 발생하지 않음 |
| TC-SUP-005 | FR-004 | sequence gap 검증 | `ESTABLISHED` 상태 session, 먼저 in-order frame 기록 | sequence가 건너뛴 frame 처리 | `RETRANSMISSION_PENDING` 전이 | supervisor가 gap을 감지해 retransmission 경로를 연다 |
| TC-SUP-006 | SR-001 | stale sequence protocol error 검증 | `ESTABLISHED` 상태 session, 먼저 in-order frame 기록 | duplicate/stale frame 처리 | `SAFE_DISCONNECT` 전이 | supervisor가 stale sequence를 protocol error로 처리한다 |
| TC-SUP-007 | FR-003 | poll receive handshake 경로 검증 | available channel, receive가 handshake frame 반환하도록 준비 | `poll_receive` 호출 | `ESTABLISHED` 전이, poll/receive count 증가 | runtime loop entry가 direct frame path와 동일 동작을 보장 |
| TC-SUP-008 | SR-002 | poll receive channel down/no-frame 검증 | unavailable channel 또는 receive unavailable 준비 | `poll_receive` 호출 | `CHANNEL_DOWN` 또는 `NO_FRAME` 반환 | channel gate가 decode/session 호출보다 우선 적용 |
