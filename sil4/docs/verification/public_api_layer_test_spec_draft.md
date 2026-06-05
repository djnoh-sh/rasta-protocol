# Test Specification Draft - Public API Layer

## Document Control

- Document ID: `TS-007`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-05-01`

## Scope

- 테스트 대상:
  - `MOD-001 Public API Layer`
- 관련 요구사항:
  - `FR-001`
  - `FR-005`
  - `IF-001`
  - `SR-004`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-API-001 | FR-001, IF-001 | session init/start/connect 경로 검증 | 유효 session config 준비 | `session_init`, `session_start`, `session_connect` 호출 | 상태가 `CONNECTING`까지 순차 전이 | callback, transport, timer 동작이 설계와 일치 |
| TC-API-016 | FR-001, IF-001 | session init report baseline 검증 | dirty last report를 가진 session context와 유효 session config 준비 | `rsrx_session_init` 호출 | session이 initialized 되고 orchestrator state는 `UNINITIALIZED`, last report transition/status/reason/diagnostic/action counts는 neutral baseline으로 clear된다 | public API startup이 이전 report/runtime state를 새 session baseline에 누출하지 않는다 |
| TC-API-002 | FR-005, SR-004 | session disconnect 경로 검증 | `ESTABLISHED` 상태 session | `session_disconnect` 호출 | `SAFE_DISCONNECT` 전이와 lifecycle callback 발생 | disconnect reason과 callback 동작이 설계와 일치 |
| TC-API-003 | IF-001 | invalid argument 방어 검증 | null session, 미초기화 session, stale report pointer | report pointer를 받는 public API guard 호출 | 정의된 invalid argument 처리와 valid report output pointer null clear | UB 없이 결정적 오류를 반환하고 caller에게 stale report pointer를 남기지 않음 |
| TC-API-004 | IF-001 | inbound heartbeat handoff 검증 | `ESTABLISHED` 상태 session | `session_process_event(VALID_HEARTBEAT)` 호출 | 상태 유지, supervision timer 재시작, diagnostic 기록 | transport 추가 송신 없이 운영 이벤트가 반영된다 |
| TC-API-005 | FR-003, IF-001 | inbound data handoff 검증 | `ESTABLISHED` 상태 session, inbound message 기록됨 | `session_process_event(VALID_DATA)` 호출 | 상태 유지, application data callback 수행 | 전달 reason, payload, sequence, confirmation과 timer/diagnostic 동작이 설계와 일치 |
| TC-API-006 | FR-004, IF-001, SR-004 | retransmission 경로 검증 | `ESTABLISHED` 상태 session | `SEQUENCE_GAP_DETECTED`, 이어서 `RECOVERY_SUCCESS` 호출 | `RETRANSMISSION_PENDING` 진입 후 `ESTABLISHED` 복귀 | retransmission 요청, API notify, recovery lifecycle가 추적 가능하게 남는다 |
| TC-API-007 | SR-002, IF-001 | supervision timer expiry 검증 | `ESTABLISHED` 상태 session | `session_process_timer_expiry(SUPERVISION)` 호출 | `SAFE_DISCONNECT` 전이와 disconnect action 발생 | timeout reason과 fail-safe action이 설계와 일치 |
| TC-API-008 | FR-004, SR-002, IF-001 | retransmission timer expiry 검증 | `RETRANSMISSION_PENDING` 상태 session | `session_process_timer_expiry(RETRANSMISSION)` 호출 | `SAFE_DISCONNECT` 전이와 disconnect action 발생 | retransmission failure reason과 action이 설계와 일치 |
| TC-API-009 | IF-001 | invalid/unsupported timer source 검증 | invalid timer source 또는 `DIAGNOSTIC_FLUSH` source | `session_process_timer_expiry` 호출 | `INVALID_ARGUMENT` 반환 | 지원 범위 밖 timer source를 결정적으로 거부한다 |
| TC-API-010 | FR-003, IF-001 | outbound application data send 검증 | `ESTABLISHED` 상태 session | `session_send_application_data` 호출 | `DATA` frame이 encode되어 transport로 전달 | reason, sequence, confirmation, payload가 설계와 일치 |
| TC-API-011 | IF-001 | outbound application data state/payload guard 검증 | `INITIALIZED` 또는 `ESTABLISHED` 상태 session | invalid state 또는 invalid payload로 `session_send_application_data` 호출 | `INVALID_STATE` 또는 `INVALID_ARGUMENT` 반환 | direct-send contract의 입력 방어가 결정적이다 |
| TC-API-012 | FR-003, IF-001, SR-003 | session reset channel-manager runtime state 검증 | active-standby session에서 pending flap penalty가 armed된 상태 | `rsrx_session_reset` 호출 후 channel-manager selection 수행 | pending penalty가 clear되고 reset-clear telemetry가 증가한다 | public API reset이 orchestrator뿐 아니라 redundancy runtime state도 deterministic하게 초기화한다 |
| TC-API-013 | FR-003, IF-001 | session reset transport-adapter runtime state 검증 | established session에서 outstanding send와 deferred send가 존재하는 상태 | `rsrx_session_reset` 호출 | outstanding/deferred outbound state와 inbound cache가 clear되고 runtime-reset telemetry가 증가하며 cumulative send/queue peak telemetry는 보존된다 | public API reset 이후 stale outbound queue/protocol runtime state가 남지 않고 reset-origin과 cumulative telemetry history가 관찰 가능하다 |
| TC-API-014 | FR-001, FR-006, SR-001, IF-001 | CRC-required session init policy gate 검증 | `uRequireCrc=1`인 session config와 default codec port 또는 CRC32 codec port 준비 | `rsrx_session_init` 호출 | default codec port는 `INVALID_ARGUMENT`로 거부되고 CRC32 codec port는 `OK` | public API startup 경계가 configuration validator의 CRC-required deployment policy를 우회하지 않는다 |
| TC-API-015 | FR-001, FR-006, SR-001, IF-001 | unavailable MAC/timestamp session init policy gate 검증 | `uRequireMac=1` 또는 `uRequireTimestamp=1`인 session config 준비 | `rsrx_session_init` 호출 | `INVALID_ARGUMENT` 반환, session은 initialized state로 진입하지 않음 | public API startup 경계가 unavailable MAC/timestamp deployment policy를 silent downgrade하지 않는다 |
| TC-API-017 | FR-003, IF-001 | session reset report baseline 검증 | supervision timeout 이후 dirty last report를 가진 initialized session | `rsrx_session_reset` 호출 | session state가 `UNINITIALIZED`로 돌아가고 last report status/reason/diagnostic/action counts가 neutral baseline으로 clear된다 | public API reset 이후 stale fail-safe report가 다음 startup/reporting cycle에 누출되지 않는다 |
| TC-API-018 | FR-001, FR-003, IF-001 | session reset 이후 restart 경로 검증 | supervision timeout 이후 reset이 완료된 initialized session | `rsrx_session_start`, `rsrx_session_connect` 재호출 | same session context가 `INITIALIZED`, `CONNECTING`으로 재진입하고 기존 callback/executor binding으로 action이 dispatch된다 | reset이 재초기화 없이 다음 startup/connect cycle을 깨뜨리지 않는다 |
| TC-API-019 | FR-003, SR-002, IF-001 | session reset timer quiescence 검증 | established session과 timer command stub 준비 | `rsrx_session_reset` 호출 | supervision 및 retransmission timer에 `CANCEL` command가 `NONE` reason과 zero deadline으로 순차 발행된다 | reset 진입 시 실행 중일 수 있는 runtime timer가 명시적으로 quiesce되어 stale timeout/retransmission expiry가 reset session에 재유입되는 위험을 줄인다 |
| TC-API-020 | IF-001, IF-002, SR-004 | public API critical-section balanced entry/exit 검증 | initialized session과 critical-section counting stub 준비 | start/connect/event/get-state/send/reset public API 호출 | 각 guarded public API 호출에서 enter/exit count가 균형을 이루고 active depth가 0으로 복귀한다 | shared session-state 접근 경로가 portable critical-section seam을 통과하며 host baseline에서 non-nested balanced 호출을 보인다 |
| TC-API-021 | IF-001, IF-002, SR-004 | public API critical-section enter failure guard 검증 | initialized session과 enter failure를 반환하는 critical-section stub 준비 | `rsrx_session_start` 호출 | `INVALID_ARGUMENT` 반환, report pointer null clear, state/callback/diagnostic side effect 없음 | lock acquisition 실패 시 public API가 상태 변경 없이 deterministic하게 거부된다 |
| TC-API-022 | IF-001, IF-002, SR-004 | outbound telemetry snapshot API 검증 | established session과 critical-section counting/failure stub 준비 | `rsrx_session_copy_outbound_telemetry` 호출 | 성공 시 telemetry 값이 caller buffer로 복사되고, enter 실패 시 caller buffer가 neutral baseline으로 clear된다 | multi-task target policy에서 내부 telemetry pointer에 의존하지 않는 bounded snapshot API를 사용할 수 있다 |
| TC-API-023 | IF-001, IF-002, SR-004 | outbound queue snapshot API 검증 | outstanding/deferred outbound state가 있는 established session과 critical-section failure stub 준비 | `rsrx_session_copy_outbound_queue_snapshot` 호출 | outstanding presence/channel, deferred state, and telemetry가 caller buffer로 복사되고, enter 실패 시 snapshot 전체가 neutral baseline으로 clear된다 | supervisor/target code가 session adapter internals를 직접 읽지 않고 bounded snapshot을 사용할 수 있다 |
| TC-API-024 | IF-001, IF-002, SR-004 | channel-manager snapshot API 검증 | active-standby channel-manager state가 있는 initialized session과 critical-section failure stub 준비 | `rsrx_session_copy_channel_manager_snapshot` 호출 | active/preferred channel, available-channel count, holdoff target/remaining, audit counters가 caller buffer로 복사되고, enter 실패 시 snapshot 전체가 neutral baseline으로 clear된다 | supervisor/target code가 channel-manager internals를 직접 읽지 않고 bounded snapshot을 사용할 수 있다 |
| TC-API-025 | IF-001, IF-002, SR-004 | inbound event resolve API guard 검증 | initialized session, decoded inbound message, critical-section counting/failure stub 준비 | `rsrx_session_resolve_inbound_event` 호출 | 성공 시 effective event가 caller buffer로 복사되고, enter 실패/null message 시 output event가 `INVALID`로 clear된다 | supervisor/target code가 protocol context internals를 직접 읽지 않고 bounded public API boundary를 사용할 수 있다 |
| TC-API-026 | IF-001, IF-002, SR-004 | inbound message record API guard 검증 | initialized session, decoded inbound data message, critical-section counting/failure stub 준비 | `rsrx_session_record_inbound_message` 호출 후 `VALID_DATA` event 처리 | enter 실패/null message는 `INVALID_ARGUMENT`로 거부되고, 성공 시 recorded message가 application callback으로 전달된다 | supervisor/target code가 transport adapter inbound-record internals를 직접 mutation하지 않고 bounded public API boundary를 사용할 수 있다 |
| TC-API-027 | IF-001, IF-002, SR-004 | outbound feedback clear API guard 검증 | outstanding send가 있는 established session과 critical-section failure stub 준비 | `rsrx_session_clear_outstanding_send_on_feedback` 호출 | enter 실패는 `INVALID_ARGUMENT`로 거부되고 outstanding state를 유지하며, 성공 시 outstanding state와 feedback-clear telemetry가 갱신된다 | supervisor/target code가 transport adapter outstanding-clear internals를 직접 mutation하지 않고 bounded public API boundary를 사용할 수 있다 |
