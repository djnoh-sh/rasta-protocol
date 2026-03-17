# SIL4 Reimplementation Roadmap Status

## Document Control

- Document ID: `PLAN-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Summary

- 현재 전체 진행률 추정: `50~55%`
- 현재 단계: `코어 흐름 연결 및 explicit application send contract 반영 단계`
- 다음 주력 단계: `transport supervisor 고도화`, `integration harness`, `redundancy/channel manager`

## Overall Phase Status

| Phase ID | Phase | Goal | Status | Progress Estimate | Notes |
| --- | --- | --- | --- | --- | --- |
| P1 | 계획/기준선 수립 | 규칙, 문서 구조, 요구사항/HLD/초기 추적성 확립 | Completed | 100% | 작업 규칙과 기본 산출물 체계 정착 |
| P2 | 코어 구조 설계/구현 | 상태 머신, orchestrator, API, abstraction, validator 뼈대 구현 | Completed | 100% | 주요 모듈 골격과 단위 테스트 확보 |
| P3 | 프로토콜 동작 구체화 | timer, sequencing, retransmission, outbound/inbound complete flow 구현 | In Progress | 35% | timer ingress, outbound encode, protocol context 초기 단계 완료 |
| P4 | 통합/강건성 검증 | integration harness, 장시간/경계/고장주입 시험 | Not Started | 0% | 아직 unit 중심 |
| P5 | 인증 증빙 강화 | static analysis, MISRA evidence, review record, safety case 입력 생성 | Not Started | 5% | 추적성 초안만 있음 |

## Workstream Status

| Workstream | Scope | Current Status | Evidence |
| --- | --- | --- | --- |
| Rules and Governance | 과제 운영 규칙, 코딩 규칙, 작업공간 기준 | Completed | `SIL4_REIMPLEMENTATION_RULES.md`, `CODING_RULES.md`, `sil4/README.md` |
| Requirements and HLD | SRS, hazard, architecture | In Progress | `system_requirements_draft.md`, `hazard_log_draft.md`, `reimplementation_architecture_draft.md` |
| Traceability | 요구사항-설계-코드-테스트 매핑 | In Progress | `traceability_matrix_initial.md` |
| State Machine | 연결 상태와 전이 규칙 | In Progress | `rsrx_state_machine.*`, `test_rsrx_state_machine.c` |
| Orchestrator | 상태 결정과 side effect dispatch 경계 | In Progress | `rsrx_orchestrator.*`, `test_rsrx_orchestrator.c` |
| Platform Abstraction | clock/timer/diagnostics 계약 | Completed | `rsrx_platform.h`, `test_rsrx_platform_contract.c` |
| Transport Abstraction | send/receive/query 계약 | Completed | `rsrx_transport.h`, `test_rsrx_transport_contract.c` |
| Adapter Layer | transport/timer/diagnostics executor binding | In Progress | `rsrx_platform_adapters.*`, `test_rsrx_platform_adapters.c` |
| Public API Session | session init/start/connect/disconnect/event API | In Progress | `rsrx_api.*`, `test_rsrx_api.c` |
| Application Data Contract | inbound data를 상위 계층 callback으로 전달 | Completed | `application_data_contract_lld_draft.md`, `application_data_contract_test_spec_draft.md`, `test_rsrx_api.c`, `test_rsrx_platform_adapters.c` |
| Codec | encode/decode skeleton | In Progress | `rsrx_codec.*`, `test_rsrx_codec.c` |
| Protocol Context | sequence/confirmation/retransmission base 관리 | In Progress | `rsrx_protocol_context.*`, `test_rsrx_protocol_context.c` |
| Transport Supervisor | inbound frame to session handoff | In Progress | `rsrx_transport_supervisor.*`, `test_rsrx_transport_supervisor.c` |
| Configuration Validation | startup gate | Completed | `rsrx_config_validator.*`, `test_rsrx_config_validator.c` |
| Integration Verification | cross-module scenario test | Not Started | 없음 |
| Safety Evidence | MISRA, static analysis, formal review records | Not Started | 없음 |

## Completed Milestones

| Milestone ID | Description | Status | Main Outputs |
| --- | --- | --- | --- |
| M1 | 프로젝트 규칙과 문서 작업공간 수립 | Completed | rules, templates, `sil4/` workspace |
| M2 | 요구사항/HLD/추적성 초안 수립 | Completed | SRS/HLD/traceability drafts |
| M3 | 상태 머신 설계 패키지와 skeleton 구현 | Completed | state machine docs, code, tests |
| M4 | orchestrator와 executor category 분리 | Completed | orchestrator docs, code, tests |
| M5 | platform/transport abstraction 계약 수립 | Completed | abstraction headers and contract tests |
| M6 | adapter layer 구현 | Completed | platform adapter code and tests |
| M7 | session public API 도입 | Completed | API layer docs, code, tests |
| M8 | codec skeleton 및 transport supervisor inbound path 도입 | Completed | codec/supervisor docs, code, tests |
| M9 | inbound failure coverage 보강 | Completed | supervisor decode failure tests |
| M10 | session inbound event coverage 보강 | Completed | heartbeat/data/retransmission tests |
| M11 | configuration validator 도입 | Completed | validator docs, code, tests |
| M12 | timer event ingress 도입 | Completed | timer expiry API, tests, traceability |
| M13 | outbound transport encode 경로 연결 | Completed | codec-backed transport adapter, tests |
| M14 | protocol context 도입 | Completed | sequence/confirmation/retransmission base context, tests |
| M15 | application data delivery contract 도입 | Completed | application callback contract, tests, traceability |
| M16 | outbound application send contract 도입 | Completed | explicit session send API, adapter helper, tests |

## In-Progress Items

| Item ID | Item | Current State | Exit Criteria |
| --- | --- | --- | --- |
| IP-001 | Public API hardening | timer ingress, outbound send, application delivery contract 포함 | queueing/backpressure policy와 callback/report semantics 결정 |
| IP-002 | Codec maturation | deterministic skeleton과 outbound encode 연결 완료 | 실제 protocol field rules, length/range checks, negative vectors 보강 |
| IP-003 | Protocol context maturation | confirmation validation, sequence gap detail, recovery success semantics, retransmission confirm rules 구현 | richer edge cases 보강 |
| IP-004 | Transport supervisor maturation | inbound decode handoff, sequence gate, poll receive, bounded pump loop, channel state gate, send result/timer delegation, send failure budget, report observability 강화 구현 | integration-facing runtime event model 반영 |
| IP-005 | Traceability enrichment | 초기 매트릭스 존재 | 모든 구현 모듈과 테스트, 리뷰 항목 연결 |

## Not-Started Items

| Item ID | Item | Why It Matters | Planned Entry Point |
| --- | --- | --- | --- |
| NS-001 | Detailed Sequence Validation | inbound/outbound sequence gap 판단과 confirm 검증이 아직 단순화돼 있음 | supervisor와 protocol context를 decoded message detail과 결합 |
| NS-002 | Transport Supervisor Completion | receive loop, send result, channel state 반영 필요 | supervisor contract 확장 |
| NS-003 | Redundancy/Channel Manager | 실제 RaSTA 특성 대응 핵심 | transport abstraction 상위 모듈 추가 |
| NS-005 | Integration Test Harness | unit만으로는 안전 시나리오 커버 불가 | fake transport/fake time 기반 harness 구축 |
| NS-006 | Static Analysis and MISRA Evidence | SIL4 과제의 핵심 증빙 | toolchain policy와 report template 수립 |
| NS-007 | Review Records and Safety Evidence | 심사 대응 산출물 필요 | review templates와 audit trail 채우기 |

## Readiness Assessment

| Area | Readiness | Comment |
| --- | --- | --- |
| 설계 구조 | High | 모듈 경계와 책임 분리는 많이 안정됨 |
| 단위 테스트 기반 | Medium | 핵심 skeleton coverage는 있으나 protocol complete 수준은 아님 |
| 프로토콜 완성도 | Medium-Low | timer ingress와 outbound encode는 연결됐지만 detailed sequencing과 redundancy가 미완 |
| 통합 가능성 | Medium | 주요 경계는 연결됐지만 end-to-end flow와 운영 루프는 미완 |
| 인증 증빙 준비 | Low | 초안 중심이며 formal evidence는 거의 없음 |

## Current Risks

| Risk ID | Risk | Impact | Mitigation Direction |
| --- | --- | --- | --- |
| R-001 | detailed sequence validation 부분 미완 | retransmission confirm rules는 있으나 richer edge cases와 confirm semantics가 아직 단순화돼 있음 | protocol context와 supervisor 규칙 확장 |
| R-002 | transport supervisor 운영 루프 부분 미완 | report observability는 강화됐으나 runtime feedback policy와 retry semantics가 아직 단순화돼 있음 | runtime feedback rule과 retry semantics 확장 |
| R-003 | redundancy 미구현 | 실제 SIL4 과제 범위 대응 부족 | channel manager 별도 workstream 시작 |
| R-004 | outbound application send가 direct-send 모델에 머묾 | 현재는 queue/backpressure/retry semantics가 없다 | queueing policy와 runtime feedback contract 설계 |
| R-005 | 인증 증빙 부족 | 코드가 있어도 심사 대응 불가 | MISRA/static analysis/review records 병행 시작 |

## Recommended Next Order

1. `Transport Supervisor Maturation`
2. `Redundancy and Channel Manager`
3. `Integration Test Harness`
4. `Static Analysis and Safety Evidence`

## Next Gate Definition

- Gate Name: `G-Protocol-Context`
- 목표:
  - inbound decoded message가 sequence/confirmation context에 반영될 것
  - outbound data/retransmission/disconnect가 sequence/confirmation을 포함해 encode될 것
  - retransmission clear와 failure semantics가 명시적 context를 기반으로 동작할 것
- 통과 조건:
  - 관련 단위 테스트와 최소 supervisor-level 테스트 추가
  - traceability matrix에 해당 경로 반영
  - HLD/LLD 갱신
