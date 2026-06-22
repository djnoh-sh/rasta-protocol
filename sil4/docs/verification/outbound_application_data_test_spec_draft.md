# Test Specification Draft - Outbound Application Data Send Contract

## Document Control

- Document ID: `TS-013`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Scope

- 테스트 대상:
  - `MOD-001 Public API Layer`
  - `MOD-009 Platform Adapter Layer`
- 관련 요구사항:
  - `FR-003`
  - `IF-001`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-OUT-001 | FR-003, IF-001 | outbound application send 성공 경로 검증 | `ESTABLISHED` 상태 session | `rsrx_session_send_application_data` 호출 | `DATA` frame이 encode되어 transport send 발생 | reason, sequence, confirmation, payload가 설계와 일치 |
| TC-OUT-005 | FR-003, IF-001 | bounded queue guard 검증 | `ESTABLISHED` 상태 session, 첫 outbound send 수행됨 | second/third/fourth/fifth/sixth/seventh/eighth/ninth/tenth/eleventh/twelfth/thirteenth send 후 fourteenth send를 호출 | second/third/fourth/fifth/sixth/seventh/eighth/ninth/tenth/eleventh/twelfth/thirteenth send는 queue에 수용되고, fourteenth send는 `REJECTED`와 API callback/diagnostic rejection을 발생 | `outstanding 1 + deferred 12`를 넘으면 overflow reject가 관찰 가능하게 남는다 |
| TC-OUT-002 | IF-001 | invalid state guard 검증 | `INITIALIZED` 상태 session | `rsrx_session_send_application_data` 호출 | `INVALID_STATE` 반환 | `ESTABLISHED` 외 상태에서 send를 거부한다 |
| TC-OUT-003 | IF-001 | invalid payload guard 검증 | `ESTABLISHED` 상태 session | null payload + nonzero length로 send 호출 | `INVALID_ARGUMENT` 반환 | 잘못된 payload 조합을 결정적으로 거부한다 |
| TC-OUT-004 | FR-003 | transport adapter direct-send encode 검증 | transport adapter 초기화 완료 | `rsrx_transport_adapter_send_application_data` 호출 | `DATA` frame encode 후 transport send 수행 | reason=`APPLICATION_DATA_REQUESTED`, payload copy, encoded length가 설계와 일치 |
| TC-OUT-006 | FR-003 | transport adapter deferred dispatch 검증 | transport adapter 초기화 완료, 첫 outbound send 수행됨 | second send 시도 후 valid inbound record, 다시 send | second send는 queue에 저장되고, inbound 후 deferred send가 dispatch되며 이후 다시 queue 수용 가능 | adapter outstanding/deferred state가 bounded queue policy와 일치한다 |
| TC-OUT-007 | FR-003 | outbound telemetry 누적 검증 | session/adapter 초기화 완료 | success, repeated busy reject, inbound clear, feedback clear 경로 수행 | accepted/busy/clear telemetry와 consecutive/max busy reject streak, escalation hit telemetry가 기대값으로 누적된다 | direct-send backpressure policy가 관찰 가능한 counter로 남는다 |
| TC-OUT-008 | FR-003 | busy reject threshold escalation 검증 | `uBusyRejectErrorThreshold=2` configured session, first outbound send 수행됨 | busy reject를 2회 연속 발생시킴 | 첫 reject는 warning, 두 번째 reject는 error diagnostic로 승격 | repeated busy reject가 configured threshold에서 escalation policy를 따른다 |
| TC-OUT-009 | FR-003, IF-001 | deferred queue FIFO dispatch 검증 | transport adapter 초기화 완료, `outstanding 1 + deferred 12` capacity 안에서 first three deferred entries가 준비됨 | valid inbound clear 후 matching clear를 두 번 더 수행 | first clear는 second payload를, second clear는 third payload를, third clear는 fourth payload를 순서대로 dispatch한다 | representative three-entry path에서도 current deferred queue가 FIFO ordering을 유지한다 |
| TC-OUT-010 | FR-003, IF-001 | deferred queue mixed clear long-run ordering 검증 | transport adapter 초기화 완료, `outstanding 1 + deferred 12` capacity 안에서 first three deferred entries로 alternating clear-source cycle을 만들 수 있는 상태 준비 | first cycle에서 manual clear 후 inbound clear 후 trailing manual clear를 수행하고, second cycle에서 inbound clear 후 manual clear 두 번을 수행 | alternating clear-source cycle 전반에서도 current deferred queue는 FIFO ordering을 유지하고 clear-source telemetry가 누적되며 peak deferred depth는 representative dispatch 이후 retain된다 | dispatch count, clear-manual count, clear-on-inbound count, last-request payload ordering, max deferred depth retain이 설계와 일치 |
| TC-OUT-011 | FR-003, IF-001 | busy reject threshold manual/inbound reset-source 검증 | transport adapter가 manual clear와 inbound clear를 모두 받을 수 있는 상태 준비 | busy reject escalation을 만든 뒤 manual clear로 한 번 reset하고, 다시 busy reject를 만든 뒤 inbound clear로 한 번 더 reset한다 | manual clear와 inbound clear는 모두 busy reject streak/latch를 reset하고, 각 reset 뒤 busy path는 현재 queue occupancy 기준에서 다시 warning 경로부터 재시작한다 | streak/latch reset, clear-source telemetry, deferred dispatch, escalation count retain, restarted streak가 설계와 일치 |
| TC-OUT-012 | FR-003, IF-001 | deferred queue telemetry accumulation 검증 | transport adapter 초기화 완료, `outstanding 1 + deferred 12` capacity 안에서 representative three-entry queue/clear cycle을 반복할 수 있는 상태 준비 | `send + queue + queue + queue + clear*4` cycle을 두 번 반복한다 | representative queue/clear cycle을 반복해도 queued count, max deferred depth, deferred dispatch count, accepted count, clear-manual count가 누적 정책과 일치한다 | cumulative queued/dispatch/accepted/manual-clear telemetry와 peak deferred depth가 설계와 일치 |
| TC-OUT-013 | FR-003, IF-001 | overflow/busy reject accumulation 검증 | transport adapter 초기화 완료, `outstanding 1 + deferred 12` saturation/clear cycle과 repeated reject를 만들 수 있는 상태 준비 | first cycle에서 overflow reject 후 clear로 streak를 reset하고, second cycle에서 overflow reject와 busy escalation 경로를 연속으로 만든 뒤 다시 clear한다 | overflow reject count와 busy reject count는 cycle을 넘어 누적되고, streak/latch는 clear에서 reset되며 escalation count는 retain되고 peak deferred depth는 saturation 이후 retain된다 | cumulative overflow/busy reject count, streak/latch reset, escalation retain, max streak, max deferred depth retain이 설계와 일치 |
| TC-OUT-017 | FR-003, IF-001 | outbound reject reason telemetry 검증 | transport adapter 초기화 완료, `outstanding 1 + deferred 12` saturation 상태 준비 | overflow reject 후 deferred dispatch와 subsequent queued submit을 수행 | overflow reject는 `QUEUE_OVERFLOW` reason을 남기고, internal deferred dispatch는 마지막 reject reason을 보존하며, subsequent accepted/queued application submit은 reason을 `NONE`으로 clear한다 | last reject reason telemetry가 overflow 원인을 보존하면서 새 application submit 성공 시 stale reason을 제거한다 |
| TC-OUT-015 | FR-003, IF-001 | outbound queue long-run representative matrix 검증 | alternating clear-source, reset-source, telemetry accumulation, overflow/busy accumulation을 모두 재현 가능한 adapter test set 준비 | mixed clear long-run, manual/inbound reset-source, telemetry accumulation, overflow/busy accumulation matrix를 representative wrapper로 실행 | current outbound queue long-run family에서 FIFO/depth parity, peak deferred depth retention, busy reject reset-source parity, overflow/busy accumulation parity가 representative wrapper 수준에서도 서로 모순 없이 유지된다 | representative wrapper가 current outbound queue long-run family parity를 별도 closeout 항목으로 추적 가능하게 유지한다 |
| TC-OUT-016 | FR-003, IF-001 | outbound queue fairness representative matrix 검증 | strict FIFO dispatch path와 alternating clear-source long-run path를 모두 재현 가능한 adapter test set 준비 | deferred queue FIFO dispatch, mixed clear long-run ordering을 representative wrapper로 실행 | current outbound queue fairness family에서 deferred dispatch는 direct clear path와 alternating clear-source long-run path 모두에서 FIFO ordering을 유지하고 peak deferred depth retain과 dispatch ordering이 서로 모순 없이 유지된다 | representative wrapper가 current outbound queue fairness family parity를 별도 closeout 항목으로 추적 가능하게 유지한다 |
| TC-OUT-014 | FR-003, IF-001 | outbound queue/backpressure closeout matrix 검증 | `outstanding 1 + deferred 12` queue, alternating clear-source, overflow/busy reject accumulation, telemetry accumulation을 모두 재현 가능한 adapter test set 준비 | mixed clear long-run, manual/inbound reset-source, telemetry accumulation, overflow/busy accumulation matrix를 closeout wrapper로 실행 | bounded queue policy의 대표 unit variant에서 FIFO/depth parity, peak deferred depth retention, busy reject reset-source parity, overflow/busy accumulation parity가 서로 모순 없이 유지된다 | representative wrapper가 outbound queue/backpressure 규칙군의 unit coverage를 하나의 closeout 항목으로 추적 가능하게 유지한다 |
