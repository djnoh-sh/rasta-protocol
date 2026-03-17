# Test Specification Draft - Platform Adapter Layer

## Document Control

- Document ID: `TS-005`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Scope

- 테스트 대상:
  - `MOD-007 Platform Adapter Layer`
- 관련 요구사항:
  - `IF-002`
  - `FR-007`
  - `SR-003`
  - `SR-004`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-PA-001 | IF-002 | platform-backed executor table 조립 검증 | 유효 port table, transport/application/api/lifecycle executor 준비 | build executor table 호출 | transport/timer/diagnostics executor가 adapter 함수로 연결되고 application executor가 보존됨 | executor table이 설계와 일치 |
| TC-PA-002 | SR-003 | transport encode 및 timer action 변환 검증 | transport send stub, codec encode 구현, monotonic clock stub 준비 | `START_HANDSHAKE`, `START_SUPERVISION_TIMER` dispatch | encoded send request 및 timer command가 생성 | message type/reason/sequence/confirmation, timer id/command/deadline이 설계와 일치 |
| TC-PA-004 | FR-003 | inbound message cache 검증 | transport adapter 초기화됨 | inbound decoded message 기록 후 조회 | 마지막 inbound message가 보존됨 | application delivery 경계가 transport send와 분리된다 |
| TC-PA-005 | FR-003 | outbound application direct-send 검증 | transport adapter 초기화됨 | `rsrx_transport_adapter_send_application_data` 호출 | `DATA` frame encode 후 transport send 수행 | reason/payload/encoded length가 설계와 일치 |
| TC-PA-003 | FR-007, SR-004 | diagnostics action 변환 검증 | diagnostics writer stub 준비 | `LOG_DIAGNOSTIC` dispatch | diagnostic record가 severity/state/status/reason 포함으로 기록 | record 필드가 설계와 일치 |
