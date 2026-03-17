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
| TC-INT-003 | SR-002 | supervision timeout fail-safe integration 검증 | handshake 완료된 session-supervisor 준비 | `supervisor_process_timer_expiry(SUPERVISION)` 호출 | `SAFE_DISCONNECT` 전이 | timeout reason, supervisor decision, lifecycle callback이 설계와 일치 |
| TC-INT-004 | SR-002 | channel down fail-safe integration 검증 | handshake 완료된 session-supervisor 준비 | `process_transport_event(CHANNEL_DOWN)` 호출 | `SAFE_DISCONNECT` 전이 | protocol error reason, supervisor decision, lifecycle callback이 설계와 일치 |
| TC-INT-005 | SR-001 | decode failure integration 검증 | handshake 완료된 session-supervisor와 손상 프레임 준비 | `supervisor_poll_receive`로 malformed frame 처리 | `DECODE_FAILED` 반환, session state 유지 | supervisor decision, processed count, callback 부재가 설계와 일치 |
| TC-INT-006 | SR-002 | send failure budget integration 검증 | handshake 완료된 session-supervisor 준비 | `process_transport_event(SEND_FAILED)`를 2회 호출 | 1회차는 budget 처리, 2회차는 `SAFE_DISCONNECT` 전이 | send failure counter, escalation reason, lifecycle callback이 설계와 일치 |
| TC-INT-007 | SR-002 | send failure budget reset integration 검증 | handshake 완료된 session-supervisor와 정상 inbound data frame 준비 | `SEND_FAILED` 후 `supervisor_poll_receive`로 정상 frame 처리 후 다시 `SEND_FAILED` 호출 | 정상 수신으로 budget reset 후 다음 실패가 다시 budget 처리됨 | budget counter reset/restart, application callback, state retention이 설계와 일치 |
| TC-INT-008 | FR-001, FR-003, IF-001 | bounded pump runtime stability 검증 | handshake/data/no-frame와 추가 data/no-frame scripted frame 준비 | `supervisor_pump_receive`를 처리/idle/처리 순서로 반복 호출 | call-local pump count와 누적 processed count가 일관되며 `ESTABLISHED` 상태 유지 | pump local/total count, callback count, last sequence가 설계와 일치 |
| TC-INT-009 | FR-001, FR-003 | bounded soak-style pump 검증 | handshake와 연속 data 5개, trailing no-frame scripted frame 준비 | `supervisor_pump_receive`를 충분한 bound로 1회 호출 | 연속 frame들이 모두 처리되고 trailing idle에서 정상 종료 | local/total processed count, iteration count, callback count, last sequence가 설계와 일치 |
| TC-INT-010 | FR-003, SR-002 | channel failover integration 검증 | active-standby channel manager와 primary/secondary fake transport 준비 | handshake 후 `CHANNEL_DOWN(primary)` 처리, outbound send, secondary inbound data 처리 | failover 후 상태 유지, outbound send가 secondary로 전환되고 secondary inbound data가 처리됨 | supervisor decision, selected channel, outbound send channel, application callback이 설계와 일치 |
| TC-INT-011 | FR-003 | preferred recovery integration 검증 | `TC-INT-010` 이후 primary restored 상태 준비 | primary 복구 후 channel query와 outbound send 수행 | preferred primary channel로 복귀하고 primary로 send 수행 | selected channel과 send channel이 preferred recovery policy와 일치 |

## Notes

- 현재 harness는 real codec과 fake transport를 결합한 happy-path, retransmission recovery, timeout fail-safe, channel-down fail-safe, channel failover, preferred recovery, decode failure, send failure budget, send failure budget reset, bounded pump stability, bounded soak smoke test까지 포함한다.
- long-run과 richer redundancy policy는 후속 integration 단계에서 확장한다.
