# Test Specification Draft - Protocol Context

## Document Control

- Document ID: `TS-011`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-16`

## Scope

- 테스트 대상:
  - `MOD-003 Protocol Context`
- 관련 요구사항:
  - `FR-003`
  - `FR-004`
  - `SR-003`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-PC-001 | FR-003 | outbound sequence progression 검증 | 초기화된 protocol context | 연속 outbound encode request 생성 | sequence가 1부터 단조 증가 | sequence numbering이 결정적으로 증가한다 |
| TC-PC-002 | FR-003 | inbound confirmation tracking 검증 | inbound decoded message 준비 | inbound record 후 outbound encode request 생성 | confirmation이 마지막 inbound sequence와 일치 | confirmation이 inbound context를 반영한다 |
| TC-PC-003 | FR-004 | retransmission request base sequence 검증 | last inbound sequence가 기록된 protocol context | retransmission request 생성 | payload가 `last_rx + 1`을 big-endian으로 포함 | retransmission 기준점이 결정적으로 계산된다 |
| TC-PC-004 | SR-003 | invalid argument 검증 | null context 또는 null request | context API 호출 | `INVALID_ARGUMENT` 반환 | UB 없이 명시적 오류 반환 |
