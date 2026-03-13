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
