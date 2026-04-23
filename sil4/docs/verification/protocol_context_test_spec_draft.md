# Test Specification Draft - Protocol Context

## Document Control

- Document ID: `TS-011`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-04-23`

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
| TC-PC-020 | FR-003, SR-003 | outbound sequence wraparound guard 검증 | `uNextTxSequenceNumber`가 `UINT32_MAX - 1`인 protocol context | 마지막 안전 encode request를 만든 뒤 다음 encode request를 다시 호출 | `UINT32_MAX - 1` sequence는 허용되고, 다음 호출은 `UINT32_MAX` boundary에서 `REJECTED`로 거부되며 counter가 유지된다 | sequence `0` wraparound가 발생하지 않고 outbound ordering ambiguity를 startup/runtime guard가 결정적으로 차단 |
| TC-PC-021 | FR-003, SR-003 | inbound sequence wraparound guard 검증 | `uLastRxSequenceNumber`가 `UINT32_MAX`인 protocol context | sequence `0` inbound sequenced message를 판정 | `PROTOCOL_ERROR`로 분류 | inbound `last_rx + 1` arithmetic wraparound가 정상 next sequence로 수용되지 않는다 |
| TC-PC-022 | FR-003, FR-004, SR-003 | retransmission base wraparound guard 검증 | `uLastRxSequenceNumber`가 `UINT32_MAX`이고 retransmission pending이 없는 protocol context | retransmission request encode request 생성 | `REJECTED`로 거부되고 pending/base/latest request tx/next tx sequence가 유지된다 | retransmission base가 sequence `0`으로 wraparound되지 않고 recovery 기준점 ambiguity가 차단된다 |
| TC-PC-002 | FR-003 | inbound confirmation tracking 검증 | inbound decoded message 준비 | inbound record 후 outbound encode request 생성 | confirmation이 마지막 inbound sequence와 일치 | confirmation이 inbound context를 반영한다 |
| TC-PC-003 | FR-004 | retransmission request base sequence 검증 | last inbound sequence가 기록된 protocol context | retransmission request 생성 | payload가 `last_rx + 1`을 big-endian으로 포함 | retransmission 기준점이 결정적으로 계산된다 |
| TC-PC-005 | FR-003 | inbound confirmation validity 검증 | outbound sequence가 일부 생성된 protocol context | confirmation이 sent-high-watermark를 넘거나 regress하는 inbound message 판정 | `PROTOCOL_ERROR`로 분류 | confirmation monotonicity와 upper bound를 강제 |
| TC-PC-006 | FR-004 | recovery success resolution 검증 | retransmission pending 상태의 protocol context | base sequence와 일치하는 inbound message 판정 | `RECOVERY_SUCCESS`로 분류 | retransmission recovery completion이 결정적으로 판정 |
| TC-PC-007 | FR-004 | unconfirmed recovery rejection 검증 | retransmission pending 상태의 protocol context | base sequence와 일치하지만 retransmission request confirmation이 부족한 inbound message 판정 | `PROTOCOL_ERROR`로 분류 | recovery success는 retransmission request가 remote에 반영된 경우에만 허용 |
| TC-PC-010 | FR-004 | retransmission ordering matrix 검증 | retransmission pending 상태의 protocol context | base sequence, unconfirmed base sequence, invalid-confirmation base sequence, higher sequence, lower sequence를 표 기반으로 순차 판정 | `RECOVERY_SUCCESS`, `PROTOCOL_ERROR`, `SEQUENCE_GAP_DETECTED`가 규칙대로 결정된다 | retransmission pending ordering 분기가 table-driven unit test로 결정적으로 닫힌다 |
| TC-PC-011 | FR-003 | steady-state ordering matrix 검증 | retransmission pending이 아닌 protocol context | next sequence, higher sequence gap, duplicate/lower sequence, invalid confirmation, regressing confirmation, monotonic confirmation equal case를 표 기반으로 순차 판정 | `VALID_DATA`, `SEQUENCE_GAP_DETECTED`, `PROTOCOL_ERROR`가 규칙대로 결정된다 | normal-path sequence/confirmation ordering 분기가 table-driven unit test로 결정적으로 닫힌다 |
| TC-PC-012 | FR-003, FR-004 | post-recovery ordering matrix 검증 | recovery success를 기록하고 retransmission pending을 clear한 protocol context | recovery 이후 next sequence, advanced monotonic confirmation, regressing confirmation, invalid confirmation, higher sequence gap을 표 기반으로 순차 판정 | `VALID_DATA`, `SEQUENCE_GAP_DETECTED`, `PROTOCOL_ERROR`가 규칙대로 결정된다 | recovery success 이후 steady-state confirmation progression과 next-sequence ordering 분기가 table-driven unit test로 결정적으로 닫힌다 |
| TC-PC-013 | FR-004 | repeated-gap retransmission progression 검증 | retransmission pending이 이미 시작된 protocol context | higher sequence repeated gap을 처리한 뒤 retransmission request를 다시 생성하고, 그 뒤 confirmed base-sequence recovery를 판정 | repeated gap 이후 follow-up retransmission request도 동일 base sequence를 유지하고, 최신 retransmission request confirmation이 충족되면 `RECOVERY_SUCCESS`가 된다 | repeated gap이 retransmission base를 흔들지 않고 follow-up retransmission request semantics를 안정적으로 유지함을 닫는다 |
| TC-PC-014 | FR-004 | repeated-gap recovery ordering matrix 검증 | repeated gap 이후 follow-up retransmission request까지 발생한 protocol context | latest retransmission request 기준의 confirmed/unconfirmed/invalid-confirmation base sequence, higher sequence, lower sequence를 표 기반으로 순차 판정 | `RECOVERY_SUCCESS`, `PROTOCOL_ERROR`, `SEQUENCE_GAP_DETECTED`가 규칙대로 결정된다 | repeated gap 이후 recovery 판정이 최신 retransmission request confirmation 기준으로 안정적으로 닫힌다 |
| TC-PC-015 | FR-003, FR-004 | repeated-gap post-recovery ordering matrix 검증 | repeated gap 이후 latest recovery success를 기록하고 retransmission pending을 clear한 protocol context | recovery 이후 next sequence, equal/latest confirmation, advanced confirmation, regressing confirmation, invalid confirmation, higher sequence gap을 표 기반으로 순차 판정 | `VALID_DATA`, `SEQUENCE_GAP_DETECTED`, `PROTOCOL_ERROR`가 규칙대로 결정된다 | repeated gap 이후 latest request confirmation semantics가 recovery 이후 steady-state confirmation progression과 함께 안정적으로 유지됨을 닫는다 |
| TC-PC-016 | FR-003, FR-004 | sequenced message family ordering matrix 검증 | initial, steady-state, retransmission pending protocol context와 `CONNECT_RESPONSE`, `HEARTBEAT`, `DATA`, `RETRANSMISSION_REQUEST` inbound family | message type family마다 first-sequence acceptance, steady-state next-sequence acceptance, duplicate rejection, retransmission base recovery, unconfirmed recovery rejection, higher-sequence gap을 표 기반으로 순차 판정 | 각 sequenced message family가 같은 ordering/confirmation/retransmission 규칙을 공유하면서도 정상 event는 각자의 suggested event로 유지된다 | `DATA`에 편중되지 않고 sequenced inbound family 전체에 동일 ordering contract가 적용됨을 결정적으로 닫는다 |
| TC-PC-017 | FR-003, FR-004 | post-recovery message family ordering matrix 검증 | recovery success를 기록하고 retransmission pending을 clear한 protocol context와 `HEARTBEAT`, `DATA`, `RETRANSMISSION_REQUEST` inbound family | family마다 recovery 이후 next-sequence acceptance, advanced confirmation, invalid/regressing confirmation rejection, higher-sequence gap을 표 기반으로 순차 판정 | recovery 이후에도 각 family가 같은 confirmation/sequence 규칙을 공유하면서 정상 event는 각자의 suggested event로 유지된다 | recovery 이후 ordering contract가 `DATA`뿐 아니라 sequenced inbound family 전체에 동일하게 유지됨을 닫는다 |
| TC-PC-018 | FR-003, FR-004 | protocol ordering closeout matrix 검증 | steady-state, retransmission pending, repeated-gap recovery, post-recovery, sequenced/unsequenced message family ordering 규칙군이 모두 준비된 protocol context test fixture | retransmission ordering, steady-state ordering, post-recovery ordering, repeated-gap progression/recovery/post-recovery ordering, sequenced message family ordering, post-recovery message family ordering, unsequenced message family pass-through matrix를 representative closeout wrapper로 실행 | protocol context의 sequence/confirmation/retransmission ordering 규칙군과 current unsequenced inbound family pass-through contract가 단일 representative wrapper에서 빠짐없이 재검증되고 서로 모순 없이 유지된다 | ordering contract가 `steady-state -> retransmission -> repeated-gap -> recovery -> post-recovery -> sequenced family -> unsequenced family` 축 전체에 걸쳐 닫혔음을 추적 가능하게 유지한다 |
| TC-PC-019 | FR-003 | unsequenced message family pass-through matrix 검증 | steady-state, post-recovery, retransmission pending protocol context와 `CONNECT_REQUEST`, `DISCONNECT`, `DIAGNOSTIC` inbound family | family마다 다양한 sequence/confirmation/protocol context 값을 가진 inbound message를 표 기반으로 순차 판정하고 record를 호출한다 | unsequenced inbound family는 sequence/confirmation ordering을 거치지 않고 각자의 suggested event를 그대로 유지하며, record 단계에서도 last rx, tx confirmation, remote confirmation, retransmission pending state를 변경하지 않는다 | current non-sequenced inbound family가 protocol ordering contract 바깥에서 deterministic pass-through semantics와 no-side-effect record semantics를 유지함을 닫는다 |
| TC-PC-008 | FR-003, SR-003 | duplicate inbound sequence rejection 검증 | handshake와 첫 data가 기록된 protocol context | 동일 sequence의 data message를 다시 판정 | `PROTOCOL_ERROR`로 분류 | stale/duplicate inbound data는 정상 data로 재수용되지 않는다 |
| TC-PC-009 | FR-003, SR-003 | initial zero sequence rejection 검증 | 아직 inbound sequence가 기록되지 않은 protocol context | sequence `0`의 첫 sequenced message를 판정 | `PROTOCOL_ERROR`로 분류 | 첫 inbound sequenced message는 `1`만 허용 |
| TC-PC-004 | SR-003 | invalid argument 검증 | null context 또는 null request | context API 호출 | `INVALID_ARGUMENT` 반환 | UB 없이 명시적 오류 반환 |
