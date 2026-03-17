# Test Specification Draft - Integration Harness

## Document Control

- Document ID: `TS-014`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Scope

- 테스트 대상:
  - `MOD-001 Public API Layer`
  - `MOD-003 Protocol Codec`
  - `MOD-004 Transport Supervisor`
  - `MOD-009 Platform Adapter Layer`
- 관련 요구사항:
  - `FR-001`
  - `FR-003`
  - `FR-004`
  - `IF-001`
- 테스트 레벨: `Integration`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-INT-001 | FR-001, FR-003, IF-001 | session + supervisor + real codec 연계 검증 | fake transport/platform과 scripted inbound frame 준비 | `session_start`, `session_connect`, `supervisor_pump_receive`, `session_send_application_data` 순서 실행 | handshake로 `ESTABLISHED` 전이, inbound data callback 발생, outbound application data send 수행 | pump processed count, application callback, outbound send request가 설계와 일치 |
| TC-INT-002 | FR-004, SR-002 | retransmission recovery integration 검증 | fake transport/platform과 handshake/gap/recovery scripted frame 준비 | `session_connect` 후 `supervisor_pump_receive` 실행 | gap에서 retransmission request가 송신되고 recovery success 후 `ESTABLISHED` 복귀 | retransmission request reason, recovery final reason, lifecycle callback이 설계와 일치 |

## Notes

- 현재 harness는 real codec과 fake transport를 결합한 happy-path 및 retransmission recovery smoke test까지 포함한다.
- error injection, long-run, retry, redundancy 시나리오는 후속 integration 단계에서 확장한다.
