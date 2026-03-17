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
| TC-INT-014 | FR-003, IF-001 | deferred queue telemetry integration 검증 | handshake 완료된 session-supervisor와 bounded outbound queue 준비 | application data 2건 전송 후 matching `SEND_COMPLETED` 처리 | deferred send가 즉시 dispatch되고 supervisor report가 refreshed queue telemetry를 보고 | decision, outstanding/deferred presence, queued count, dispatch count, overflow count가 설계와 일치 |
| TC-INT-015 | FR-003, IF-001 | queue overflow reject integration 검증 | handshake 완료된 session-supervisor와 bounded outbound queue 준비 | application data 3건 연속 전송 | third send가 overflow reject되고 API callback, warning diagnostic, adapter telemetry가 함께 남는다 | reject status, warning diagnostic, queued/overflow/busy counters가 설계와 일치 |
| TC-INT-016 | FR-003, IF-001 | busy reject threshold escalation integration 검증 | `uBusyRejectErrorThreshold=2` configured handshake-complete session-supervisor 준비 | application data 4건 연속 전송 | third send는 warning reject, fourth send는 error diagnostic reject로 승격 | warning->error escalation, severity, streak, escalation count/latch가 설계와 일치 |
| TC-INT-017 | SR-002 | receive error budget integration 검증 | handshake 완료된 session-supervisor와 `RX_ERROR` scripted transport 준비 | `supervisor_poll_receive`를 2회 호출 | 첫 오류는 budget 처리되어 상태 유지, 두 번째 오류는 `PROTOCOL_ERROR`로 escalation되어 `SAFE_DISCONNECT` 전이 | receive error count/reset count, escalation decision, protocol error reason, lifecycle callback이 설계와 일치 |
| TC-INT-018 | SR-002 | receive error budget reset integration 검증 | handshake 완료된 session-supervisor와 `RX_ERROR` 및 정상 data scripted transport 준비 | `RX_ERROR` 후 정상 data frame 처리, 다시 `RX_ERROR` 처리 | 정상 수신으로 receive error budget이 reset되고 다음 오류가 다시 첫 오류처럼 budget 처리됨 | receive error count/reset count, application callback, state retention, budget restart가 설계와 일치 |
| TC-INT-024 | SR-002 | mixed transient budget reset integration 검증 | handshake 완료된 session-supervisor와 `SEND_FAILED`, `RX_ERROR`, 정상 data scripted transport 준비 | send failure와 receive error를 각각 1회 발생시킨 뒤 정상 data frame 처리, 이후 두 fault를 다시 1회씩 발생 | 정상 수신으로 send/receive budget이 동시에 reset되고 이후 fault가 각각 첫 오류처럼 재시작됨 | send/receive reset count, application callback, lifecycle callback 부재, state retention이 설계와 일치 |
| TC-INT-019 | FR-003, SR-003 | duplicate inbound protocol error integration 검증 | handshake 완료 후 첫 valid data가 처리된 session-supervisor와 duplicate data scripted transport 준비 | 동일 data frame을 연속 수신 | 첫 data는 전달되고 두 번째 duplicate는 `PROTOCOL_ERROR`로 처리되어 `SAFE_DISCONNECT` 전이 | processed count, protocol error reason, application callback count, lifecycle callback이 설계와 일치 |
| TC-INT-020 | FR-003, SR-003 | invalid confirmation protocol error integration 검증 | handshake 후 outbound send가 발생한 session-supervisor와 invalid confirmation data frame 준비 | sent-high-watermark를 초과하는 confirmation을 가진 inbound data를 수신 | inbound frame이 `PROTOCOL_ERROR`로 처리되어 `SAFE_DISCONNECT` 전이 | effective event, protocol error reason, application callback 부재, lifecycle callback이 설계와 일치 |
| TC-INT-021 | FR-003, SR-003 | regressing confirmation protocol error integration 검증 | handshake 후 유효 confirmation이 한 번 기록된 session-supervisor와 regressing confirmation data frame 준비 | valid confirmation data 수신 후 더 작은 confirmation의 inbound data를 수신 | regressing confirmation frame이 `PROTOCOL_ERROR`로 처리되어 `SAFE_DISCONNECT` 전이 | prior valid callback 유지, second callback 부재, protocol error reason, lifecycle callback이 설계와 일치 |
| TC-INT-022 | FR-004, SR-003 | unconfirmed recovery protocol error integration 검증 | handshake 후 gap과 base-sequence recovery frame이 scripted된 session-supervisor 준비 | gap frame으로 retransmission pending 진입 후 confirmation이 부족한 base-sequence frame 수신 | recovery success로 복귀하지 않고 `PROTOCOL_ERROR`로 `SAFE_DISCONNECT` 전이 | retransmission request 송신, protocol error reason, application callback 부재, lifecycle callback이 설계와 일치 |
| TC-INT-023 | FR-004, SR-003 | stale retransmission protocol error integration 검증 | handshake 후 gap과 stale lower-sequence frame이 scripted된 session-supervisor 준비 | gap frame으로 retransmission pending 진입 후 `base`보다 낮은 stale frame 수신 | stale frame이 `PROTOCOL_ERROR`로 처리되어 `SAFE_DISCONNECT` 전이 | retransmission request 송신, protocol error reason, application callback 부재, lifecycle callback이 설계와 일치 |
| TC-INT-002 | FR-004, SR-002 | retransmission recovery integration 검증 | fake transport/platform과 handshake/gap/recovery scripted frame 준비 | `session_connect` 후 `supervisor_pump_receive` 실행 | gap에서 retransmission request가 송신되고 recovery success 후 `ESTABLISHED` 복귀 | retransmission request reason, recovery final reason, lifecycle callback이 설계와 일치 |
| TC-INT-003 | SR-002 | supervision timeout fail-safe integration 검증 | handshake 완료된 session-supervisor 준비 | `supervisor_process_timer_expiry(SUPERVISION)` 호출 | `SAFE_DISCONNECT` 전이 | timeout reason, supervisor decision, lifecycle callback이 설계와 일치 |
| TC-INT-004 | SR-002 | channel down fail-safe integration 검증 | handshake 완료된 session-supervisor 준비 | `process_transport_event(CHANNEL_DOWN)` 호출 | `SAFE_DISCONNECT` 전이 | protocol error reason, supervisor decision, lifecycle callback이 설계와 일치 |
| TC-INT-005 | SR-001 | decode failure integration 검증 | handshake 완료된 session-supervisor와 손상 프레임 준비 | `supervisor_poll_receive`로 malformed frame 처리 | `DECODE_FAILED` 반환, session state 유지 | supervisor decision, processed count, callback 부재가 설계와 일치 |
| TC-INT-006 | SR-002 | send failure budget integration 검증 | handshake 완료된 session-supervisor 준비 | `process_transport_event(SEND_FAILED)`를 2회 호출 | 1회차는 budget 처리, 2회차는 `SAFE_DISCONNECT` 전이 | send failure counter, escalation reason, lifecycle callback이 설계와 일치 |
| TC-INT-007 | SR-002 | send failure budget reset integration 검증 | handshake 완료된 session-supervisor와 정상 inbound data frame 준비 | `SEND_FAILED` 후 `supervisor_poll_receive`로 정상 frame 처리 후 다시 `SEND_FAILED` 호출 | 정상 수신으로 budget reset 후 다음 실패가 다시 budget 처리됨 | budget counter reset/restart, application callback, state retention이 설계와 일치 |
| TC-INT-008 | FR-001, FR-003, IF-001 | bounded pump runtime stability 검증 | handshake/data/no-frame와 추가 data/no-frame scripted frame 준비 | `supervisor_pump_receive`를 처리/idle/처리 순서로 반복 호출 | call-local pump count와 누적 processed count가 일관되며 `ESTABLISHED` 상태 유지 | pump local/total count, callback count, last sequence가 설계와 일치 |
| TC-INT-009 | FR-001, FR-003 | bounded soak-style pump 검증 | handshake와 연속 data 5개, trailing no-frame scripted frame 준비 | `supervisor_pump_receive`를 충분한 bound로 1회 호출 | 연속 frame들이 모두 처리되고 trailing idle에서 정상 종료 | local/total processed count, iteration count, callback count, last sequence가 설계와 일치 |
| TC-INT-010 | FR-003, SR-002 | channel failover integration 검증 | active-standby channel manager와 primary/secondary fake transport 준비 | handshake 후 `CHANNEL_DOWN(primary)` 처리, outbound send, secondary inbound data 처리 | failover 후 상태 유지, outbound send가 secondary로 전환되고 secondary inbound data가 처리됨 | supervisor decision, selected channel, outbound send channel, application callback, switch telemetry가 설계와 일치 |
| TC-INT-011 | FR-003 | preferred recovery integration 검증 | `TC-INT-010` 이후 primary restored 상태 준비 | primary 복구 후 channel query와 outbound send 수행 | preferred primary channel로 복귀하고 primary로 send 수행 | selected channel과 send channel이 preferred recovery policy와 일치 |
| TC-INT-012 | FR-003 | recovery holdoff integration 검증 | active-standby holdoff `2` config와 failover 완료 상태 준비 | primary 복구 후 query/send를 2회 수행 | 첫 회차는 secondary 유지, 두 번째 회차에서 primary 복귀 | holdoff 이전/이후의 selected channel, send channel, cumulative switch count가 설계와 일치 |
| TC-INT-013 | FR-003, SR-002 | redundancy flap soak integration 검증 | active-standby holdoff `2` config, handshake 완료 상태, primary up/down 반복 준비 | `CHANNEL_DOWN(primary)`와 recovery query를 반복 수행 | flap 동안 holdoff가 매 회차 다시 적용되고 충분한 안정 관측 후에만 primary로 복귀 | repeated failover/recovery cycle 동안 selected channel, outbound send channel, cumulative switch count, state retention이 설계와 일치 |
| TC-INT-025 | FR-003, SR-002 | failover transient recovery integration 검증 | active-standby session-supervisor와 secondary available 상태 준비 | primary failover 후 secondary에서 `SEND_FAILED`, `RX_ERROR`, 정상 data, outbound send 순서 실행 | secondary 경로의 transient send/receive fault가 정상 inbound data로 함께 해소되고 session은 `ESTABLISHED` 유지 | selected channel, send/receive budget reset, application callback, secondary send routing이 설계와 일치 |

## Notes

- 현재 harness는 real codec과 fake transport를 결합한 happy-path, retransmission recovery, unconfirmed recovery rejection, stale retransmission rejection, timeout fail-safe, channel-down fail-safe, channel failover, preferred recovery, recovery holdoff, failover transient recovery, redundancy flap soak, decode failure, send failure budget, send failure budget reset, receive error budget, receive error budget reset, mixed transient budget reset, duplicate inbound protocol error, invalid confirmation protocol error, regressing confirmation protocol error, bounded pump stability, bounded soak smoke test까지 포함하며 redundancy switch telemetry도 함께 검증한다.
- long-run과 richer redundancy policy는 후속 integration 단계에서 확장한다.
