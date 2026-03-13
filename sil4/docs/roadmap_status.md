# SIL4 Reimplementation Roadmap Status

## Document Control

- Document ID: `PLAN-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Summary

- 현재 전체 진행률 추정: `30~35%`
- 현재 단계: `코어 구조 구현 및 단위 검증 단계`
- 다음 주력 단계: `timer event ingress`, `outbound send path 통합`, `sequence/retransmission context 구체화`

## Overall Phase Status

| Phase ID | Phase | Goal | Status | Progress Estimate | Notes |
| --- | --- | --- | --- | --- | --- |
| P1 | 계획/기준선 수립 | 규칙, 문서 구조, 요구사항/HLD/초기 추적성 확립 | Completed | 100% | 작업 규칙과 기본 산출물 체계 정착 |
| P2 | 코어 구조 설계/구현 | 상태 머신, orchestrator, API, abstraction, validator 뼈대 구현 | In Progress | 80% | 주요 모듈 골격과 단위 테스트 확보 |
| P3 | 프로토콜 동작 구체화 | timer, sequencing, retransmission, outbound/inbound complete flow 구현 | Not Started | 10% | 일부 inbound skeleton만 존재 |
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
| Codec | encode/decode skeleton | In Progress | `rsrx_codec.*`, `test_rsrx_codec.c` |
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

## In-Progress Items

| Item ID | Item | Current State | Exit Criteria |
| --- | --- | --- | --- |
| IP-001 | Public API hardening | 기본 경로와 inbound event coverage 확보 | invalid sequencing, timer ingress, outbound send contract까지 포함 |
| IP-002 | Codec maturation | deterministic skeleton 존재 | 실제 protocol field rules, length/range checks, negative vectors 보강 |
| IP-003 | Transport supervisor maturation | inbound decode handoff만 구현 | receive loop, send result, channel state, timer interaction 반영 |
| IP-004 | Traceability enrichment | 초기 매트릭스 존재 | 모든 구현 모듈과 테스트, 리뷰 항목 연결 |

## Not-Started Items

| Item ID | Item | Why It Matters | Planned Entry Point |
| --- | --- | --- | --- |
| NS-001 | Timer Event Ingress | timeout supervision과 periodic handling의 실제 진입점 필요 | session/orchestrator에 timer source contract 추가 |
| NS-002 | Outbound Send Path Completion | API/상태 머신 action이 codec을 거쳐 실제 frame으로 송신되어야 함 | transport supervisor 또는 dedicated TX path 설계 |
| NS-003 | Sequence/Confirmation Context | ordered delivery와 retransmission의 실질 상태가 아직 없음 | protocol/session context 확장 |
| NS-004 | Redundancy/Channel Manager | 실제 RaSTA 특성 대응 핵심 | transport abstraction 상위 모듈 추가 |
| NS-005 | Application Data Contract | 상위 계층 데이터 ingress/egress 정의 필요 | API/codec/transport 경계 보강 |
| NS-006 | Integration Test Harness | unit만으로는 안전 시나리오 커버 불가 | fake transport/fake time 기반 harness 구축 |
| NS-007 | Static Analysis and MISRA Evidence | SIL4 과제의 핵심 증빙 | toolchain policy와 report template 수립 |
| NS-008 | Review Records and Safety Evidence | 심사 대응 산출물 필요 | review templates와 audit trail 채우기 |

## Readiness Assessment

| Area | Readiness | Comment |
| --- | --- | --- |
| 설계 구조 | High | 모듈 경계와 책임 분리는 많이 안정됨 |
| 단위 테스트 기반 | Medium | 핵심 skeleton coverage는 있으나 protocol complete 수준은 아님 |
| 프로토콜 완성도 | Low | sequencing, timer ingress, redundancy가 미완 |
| 통합 가능성 | Medium | 구조는 연결됐지만 end-to-end flow는 미완 |
| 인증 증빙 준비 | Low | 초안 중심이며 formal evidence는 거의 없음 |

## Current Risks

| Risk ID | Risk | Impact | Mitigation Direction |
| --- | --- | --- | --- |
| R-001 | timer event ingress 부재 | timeout 기반 안전 동작 완결 불가 | timer source contract와 tests 추가 |
| R-002 | outbound path 미완결 | API event가 wire frame 송신까지 닿지 않음 | codec encode + TX path 통합 |
| R-003 | sequence context 미부재 | ordered delivery/retransmission 요구 미충족 | session/protocol context 모델 도입 |
| R-004 | redundancy 미구현 | 실제 SIL4 과제 범위 대응 부족 | channel manager 별도 workstream 시작 |
| R-005 | 인증 증빙 부족 | 코드가 있어도 심사 대응 불가 | MISRA/static analysis/review records 병행 시작 |

## Recommended Next Order

1. `Timer Event Ingress`
2. `Outbound Send Path Completion`
3. `Sequence and Retransmission Context`
4. `Transport Supervisor Maturation`
5. `Redundancy and Channel Manager`
6. `Integration Test Harness`
7. `Static Analysis and Safety Evidence`

## Next Gate Definition

- Gate Name: `G-Next-Core-Flow`
- 목표:
  - timer expiry event가 session/orchestrator/state machine까지 결정적으로 전달될 것
  - outbound action이 codec encode를 거쳐 transport send request/frame까지 연결될 것
  - sequence gap/recovery가 명시적 context를 기반으로 동작할 것
- 통과 조건:
  - 관련 단위 테스트와 최소 통합 테스트 추가
  - traceability matrix에 해당 경로 반영
  - HLD/LLD 갱신
