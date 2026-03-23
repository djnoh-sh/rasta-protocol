# Test Specification Draft - Transport Supervisor

## Document Control

- Document ID: `TS-009`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`
 - Last Updated: `2026-03-20`

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
| TC-SUP-001 | FR-003, FR-004 | inbound frame handoff 검증 | `CONNECTING` 상태 session, codec decode stub 준비 | `FRAME_RECEIVED` frame 처리 | decoded event가 session에 전달되어 `ESTABLISHED` 전이 | decoded message, effective event, session status, decision이 설계와 일치 |
| TC-SUP-002 | SR-002 | invalid argument 방어 검증 | null context 또는 invalid frame | supervisor init/process 호출 | `INVALID_ARGUMENT` 반환 | UB 없이 결정적 오류 처리 |
| TC-SUP-003 | SR-002 | decode failure 방어 검증 | `CONNECTING` 상태 session, codec이 `DECODE_ERROR` 반환하도록 준비 | `FRAME_RECEIVED` frame 처리 | `DECODE_FAILED` 반환, session 미호출, processed count 미증가 | session state가 유지되고 supervisor report가 failure decision을 보존 |
| TC-SUP-004 | FR-003, SR-002 | unsupported message 방어 검증 | `CONNECTING` 상태 session, codec이 `UNSUPPORTED_MESSAGE` 반환하도록 준비 | 지원하지 않는 message frame 처리 | `DECODE_FAILED` 반환, decoded type은 기록되지만 session 미호출 | unsupported message가 보고서에 남고 상태 전이는 발생하지 않음 |
| TC-SUP-005 | FR-004 | sequence gap 검증 | `ESTABLISHED` 상태 session, 먼저 in-order frame 기록 | sequence가 건너뛴 frame 처리 | `RETRANSMISSION_PENDING` 전이 | supervisor가 gap을 감지해 retransmission 경로를 연다 |
| TC-SUP-006 | SR-001 | stale sequence protocol error 검증 | `ESTABLISHED` 상태 session, 먼저 in-order frame 기록 | duplicate/stale frame 처리 | `SAFE_DISCONNECT` 전이 | supervisor가 stale sequence를 protocol error로 처리한다 |
| TC-SUP-007 | FR-003 | poll receive handshake 경로 검증 | available channel, receive가 handshake frame 반환하도록 준비 | `poll_receive` 호출 | `ESTABLISHED` 전이, poll/receive count 증가 | runtime loop entry가 direct frame path와 동일 동작을 보장 |
| TC-SUP-008 | SR-002 | poll receive channel down/no-frame 검증 | unavailable channel 또는 receive unavailable 준비 | `poll_receive` 호출 | `CHANNEL_DOWN` 또는 `NO_FRAME` 반환 | channel gate가 decode/session 호출보다 우선 적용되고 decision이 보고된다 |
| TC-SUP-020 | SR-002 | poll receive transient error budget 검증 | available channel, generic receive error 준비 | `poll_receive` 호출 | 첫 receive error는 ignored event로 budget 처리 | transient receive error가 즉시 fail-safe로 확대되지 않고 decision/telemetry가 남는다 |
| TC-SUP-021 | SR-002 | poll receive repeated error escalation 검증 | `ESTABLISHED` 상태 session, generic receive error 반복 준비 | `poll_receive`를 연속 호출 | budget 도달 시 `PROTOCOL_ERROR`로 fail-safe 전이 | receive error budget reset, effective event, session status, error class telemetry가 설계와 일치 |
| TC-SUP-013 | FR-003, SR-002 | bounded pump receive drain 검증 | `ESTABLISHED` 상태 session, scripted receive/decode 준비 | `pump_receive(max_polls)` 호출 | 여러 frame 처리 후 `NO_FRAME`에서 정상 종료 | pump iteration/frame count와 terminal decision이 설계와 일치 |
| TC-SUP-014 | SR-002 | bounded pump invalid argument 검증 | null context 또는 zero max poll | `pump_receive` 호출 | `INVALID_ARGUMENT` 반환 | bounded runtime loop entry가 결정적으로 방어된다 |
| TC-SUP-009 | SR-002 | transport send failure budget 검증 | `ESTABLISHED` 상태 session 준비 | `SEND_FAILED` transport event를 연속 처리 | 첫 실패는 ignored, budget 도달 시 `SAFE_DISCONNECT` 전이 | transient failure와 persistent failure를 구분하고 decision/effective event, budget update/reset telemetry를 보고 |
| TC-SUP-010 | FR-003 | transport send completed ignored 검증 | `INITIALIZED` 상태 session 준비, outstanding send 없음 | `SEND_COMPLETED` transport event 처리 | `IGNORED_EVENT` 반환, 상태 유지 | correlation 없는 transport feedback은 stale event로 무시되고 decision이 남는다 |
| TC-SUP-018 | FR-003 | transport send completed correlation 검증 | `CONNECTING` 상태 session 준비, connect request 송신으로 outstanding send 존재 | matching `SEND_COMPLETED` transport event 처리 | `IGNORED_EVENT` 반환, outstanding send clear | correlation이 맞는 완료 feedback만 정상 소거로 처리되고 이후 stale completion과 구분된다 |
| TC-SUP-019 | FR-003 | deferred queue telemetry 검증 | outstanding send 1건과 deferred send 1건이 존재하는 session 준비 | matching `SEND_COMPLETED` transport event 처리 | deferred send가 즉시 dispatch되고 supervisor report가 refreshed queue 상태를 보고 | `uOutstandingSendPresent=1`, `uDeferredSendPresent=0`, `uQueuedSendCount=1`, `uDeferredDispatchCount=1`, `uQueueOverflowRejectCount=0` |
| TC-SUP-015 | FR-003, SR-002 | channel down failover 검증 | `ESTABLISHED` 상태 session, active-standby channel manager, primary down/secondary up 상태 준비 | `CHANNEL_DOWN(primary)` transport event 처리 후 inactive primary의 `SEND_FAILED` 처리 | `IGNORED_EVENT` 반환, active channel이 secondary로 전환되고 stale primary failure는 budget에 반영되지 않음 | session state 유지, selected channel, failover decision, stale send failure ignore policy, send failure budget reset telemetry, supervisor channel switch telemetry가 설계와 일치 |
| TC-SUP-017 | FR-003, SR-002 | channel up refresh 검증 | `ESTABLISHED` 상태 session, failover로 secondary active 상태, primary restored 상태 준비 | `CHANNEL_UP(primary)` transport event 처리 후 stale secondary `SEND_FAILED`, recovered primary `SEND_FAILED` 처리 | `IGNORED_EVENT` 반환, active selection이 refresh되어 primary 복귀하고 recovered primary failure는 fresh budget으로 처리됨 | session state 유지, selected channel, decision telemetry, switch telemetry, budget channel telemetry가 설계와 일치 |
| TC-SUP-012 | SR-002 | send failure budget reset 검증 | `ESTABLISHED` 상태 session 준비 | send failure 후 정상 inbound frame 처리, 다시 send failure 처리 | budget이 reset되어 다시 ignored 처리 | 정상 traffic이 누적 send failure를 해소하고 budget reset telemetry가 설계와 일치 |
| TC-SUP-022 | FR-003, SR-002 | budget scope matrix 검증 | `ESTABLISHED` 상태 active-standby session, secondary failover/primary recovery 가능 상태 준비 | secondary correlated `SEND_FAILED`, primary recovery, stale secondary `SEND_FAILED`, primary correlated `SEND_FAILED`, primary `RX_ERROR`, secondary failover, secondary `RX_ERROR`를 순차 처리 | send failure budget은 channel-scoped로 stale inactive-channel feedback에 영향받지 않고 channel switch 시 reset-and-increment 되며, receive error budget은 channel switch를 넘어 carry-over되어 다음 오류에서 escalation된다 | budget channel/update telemetry, send/receive counter reset, receive escalation decision, final fail-safe state가 설계와 일치 |
| TC-SUP-023 | FR-003, SR-002 | send feedback ordering matrix 검증 | `ESTABLISHED` 상태 active-standby session, secondary failover/primary recovery 가능 상태와 outstanding send correlation 준비 | `secondary send -> primary recovery -> inactive secondary SEND_FAILED -> inactive secondary SEND_COMPLETED -> primary send -> primary SEND_FAILED`를 순차 처리 | inactive secondary `SEND_FAILED`는 outstanding가 있어도 active channel이 아니므로 ignored되고 outstanding/budget을 유지하며, inactive secondary `SEND_COMPLETED`는 outstanding correlation만으로 clear를 수행하고, 이후 primary correlated `SEND_FAILED`만 primary budget `1`로 반영된다 | decision kind, outstanding clear semantics, budget channel/update telemetry, final established state가 설계와 일치 |
| TC-SUP-024 | FR-003, SR-002 | channel event ordering matrix 검증 | `ESTABLISHED` 상태 active-standby session, failover와 recovery refresh가 가능한 channel 상태 준비 | `CHANNEL_DOWN(primary) -> CHANNEL_UP(secondary no-op) -> CHANNEL_UP(primary recovery) -> CHANNEL_UP(primary repeated no-op)`를 순차 처리 | failover와 preferred primary recovery refresh는 모두 actual selection switch로 기록되고, no-op refresh는 decision은 남기되 switch count를 증가시키지 않는다 | selected channel, decision type, `uChannelSwitchCount`, `uLastChannelSwitchOccurred`, final established state가 설계와 일치 |
| TC-SUP-025 | FR-003, SR-002 | timer delegation matrix 검증 | `CONNECTING`, `ESTABLISHED`, `RETRANSMISSION_PENDING` 상태 session과 invalid timer source 구성이 가능한 supervisor 준비 | `SUPERVISION` expiry를 `CONNECTING`/`ESTABLISHED`에서, `RETRANSMISSION` expiry를 `RETRANSMISSION_PENDING`에서, `INVALID` source를 `ESTABLISHED`에서 각각 처리 | supervision expiry는 `TIMEOUT_EXPIRED` fail-safe로 위임되고, retransmission expiry는 `RETRANSMISSION_FAILED` fail-safe로 위임되며, invalid source는 session state를 유지한 채 `SESSION_ERROR`로 반환된다 | timer source별 session status, reason, supervisor decision, accepted/error decision counter, final session state가 설계와 일치 |
| TC-SUP-026 | FR-003, SR-002 | poll receive retry ordering matrix 검증 | `ESTABLISHED` 상태 session, `query/receive` status와 valid inbound frame을 단계별로 제어할 수 있는 supervisor 준비 | `RX_ERROR -> UNAVAILABLE(no frame) -> RX_ERROR -> VALID_DATA -> unavailable channel query` 순서로 `poll_receive`를 호출 | generic receive error는 budget을 증가시키지만, no-frame/valid inbound/channel-gated path는 retry budget을 reset하며, success path는 application callback까지 정상 수행한다 | receive error count/reset telemetry, decision type, success callback, final channel-gated status가 설계와 일치 |
| TC-SUP-027 | FR-003, SR-002 | pump receive terminal ordering matrix 검증 | `ESTABLISHED` 상태 session에서 scripted receive/no-frame/channel-gated 종료 조건을 구분해 준비 | `FRAME_RECEIVED -> UNAVAILABLE`, immediate `UNAVAILABLE`, immediate unavailable channel query 각각에 대해 `pump_receive(max_polls)` 호출 | processed frame이 있는 뒤 `NO_FRAME`이 오면 `OK`로 종료하고, 첫 iteration에서 바로 `NO_FRAME`이면 `NO_FRAME`, 첫 iteration에서 channel-gated면 `CHANNEL_DOWN`으로 종료한다 | terminal status, `uLastPumpIterationCount`, `uLastPumpProcessedFrameCount`, terminal decision이 설계와 일치 |
| TC-SUP-028 | FR-003, SR-002 | pump receive error ordering matrix 검증 | `ESTABLISHED` 상태 session에서 scripted receive/channel-down/decode-failure 종료 조건을 구분해 준비 | `FRAME_RECEIVED -> CHANNEL_DOWN`, `FRAME_RECEIVED -> DECODE_FAILED` 각각에 대해 `pump_receive(max_polls)` 호출 | processed frame이 먼저 있었더라도 뒤따르는 `CHANNEL_DOWN`과 `DECODE_FAILED`는 `OK`로 흡수되지 않고 terminal error로 그대로 반환된다 | terminal status, `uLastPumpIterationCount`, `uLastPumpProcessedFrameCount`, terminal decision이 설계와 일치 |
| TC-SUP-029 | FR-003, SR-002 | pump receive ignored ordering matrix 검증 | `ESTABLISHED` 상태 session에서 immediate/processed-then-budgeted receive error 경로를 구분해 준비 | immediate `RX_ERROR`, `FRAME_RECEIVED -> RX_ERROR` 각각에 대해 `pump_receive(max_polls)` 호출 | `pump_receive`는 `NO_FRAME`과 달리 `IGNORED_EVENT`를 terminal로 즉시 반환하며, 이미 처리한 frame이 있으면 processed count를 보존한 채 budgeted decision을 보고한다 | terminal status, `uLastPumpIterationCount`, `uLastPumpProcessedFrameCount`, receive-error budgeted decision이 설계와 일치 |
| TC-SUP-011 | SR-002 | timer expiry delegation 검증 | `CONNECTING` 상태 session 준비 | supervision timer expiry 처리 | `SAFE_DISCONNECT` 전이 | timer ingress가 supervisor 경계를 통해 session으로 전달되고 session status가 보고된다 |
| TC-SUP-016 | FR-003, SR-002 | decision class telemetry 검증 | representative accepted/error/ignored/rejected path 준비 | handshake, decode failure, failover ignored, channel up refresh, send failure escalation, timer delegation 경로 수행 | 각 경로가 expected decision class와 cumulative counter를 보고 | decision class mapping과 누적 카운터가 설계와 일치 |
