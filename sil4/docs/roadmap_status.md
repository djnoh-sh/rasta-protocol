# SIL4 Reimplementation Roadmap Status

## Document Control

- Document ID: `PLAN-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-18`

## Summary

- 현재 전체 진행률 추정: `91~93%`
- 현재 단계: `P3/P4 protocol hardening 단계`
- 다음 주력 단계: `detailed sequence validation`, `runtime fault integration`, `evidence maintenance`

## Overall Phase Status

| Phase ID | Phase | Goal | Status | Progress Estimate | Notes |
| --- | --- | --- | --- | --- | --- |
| P1 | 계획/기준선 수립 | 규칙, 문서 구조, 요구사항/HLD/초기 추적성 확립 | Completed | 100% | 작업 규칙과 기본 산출물 체계 정착 |
| P2 | 코어 구조 설계/구현 | 상태 머신, orchestrator, API, abstraction, validator 뼈대 구현 | Completed | 100% | 주요 모듈 골격과 단위 테스트 확보 |
| P3 | 프로토콜 동작 구체화 | timer, sequencing, retransmission, outbound/inbound complete flow 구현 | In Progress | 86% | timer ingress, outbound encode, protocol context edge case, repeated gap/recovery, retransmission timeout, failover recovery/timeout, failover repeated-gap recovery/timeout, failover stale rejection, failover unconfirmed recovery rejection, failover repeated-gap unconfirmed recovery rejection, failover invalid/regressing confirmation ordering, retransmission channel-up holdoff recovery/timeout 경로까지 반영됐지만 confirm/retransmission ordering의 richer variant는 아직 남아 있음 |
| P4 | 통합/강건성 검증 | integration harness, 장시간/경계/고장주입 시험 | In Progress | 94% | mixed transient, failover/recovery/holdoff, channel-up flap reset, repeated gap/retransmission timeout, failover recovery/timeout, receive-error failover carryover, send-failure budget failover reset, preferred recovery send-budget isolation, preferred recovery receive-error carryover/reset, preferred recovery mixed transient reset, holdoff send-budget isolation, holdoff stale feedback isolation, holdoff stale feedback recovery ordering/soak, holdoff receive-error carryover, holdoff mixed transient reset, holdoff flap transient asymmetry, failover repeated-gap recovery/timeout, failover stale rejection, failover unconfirmed recovery rejection, failover repeated-gap unconfirmed recovery rejection, failover invalid/regressing confirmation integration, redundancy flap transient soak, redundancy flap transient long-run soak, channel-up holdoff transient soak, retransmission channel-up holdoff recovery/timeout까지 확보했지만 longer-run redundancy와 richer runtime fault ordering은 아직 남아 있음 |
| P5 | 인증 증빙 강화 | static analysis, MISRA evidence, review record, safety case 입력 생성 | In Progress | 94% | baseline 증빙 체계, cppcheck cleanup, outbound queue policy review, CI linkage, severity mapping, artifact retention, MISRA subset severity, tool-specific mapping, second-tool candidate와 clang first-run baseline, CI subset summary, vendor rule mapping draft, ctest strategy baseline, PR annotation policy, vendor-aware report template, vendor matrix sample, PR annotation helper, vendor deviation example, PR comment API linkage, delta policy baseline, delta-aware helper, first real vendor onboarding procedure, baseline persistence source policy, baseline artifact fetch 구현, first actual vendor rule entry sample, audit trail closeout baseline 확보. 다만 first workflow baseline fetch success evidence와 first actual vendor evidence set은 아직 미완 |

## Workstream Status

| Workstream | Scope | Current Status | Evidence |
| --- | --- | --- | --- |
| Rules and Governance | 과제 운영 규칙, 코딩 규칙, 작업공간 기준 | Completed | `SIL4_REIMPLEMENTATION_RULES.md`, `CODING_RULES.md`, `sil4/README.md` |
| Requirements and HLD | SRS, hazard, architecture | In Progress | `system_requirements_draft.md`, `hazard_log_draft.md`, `reimplementation_architecture_draft.md` |
| Traceability | 요구사항-설계-코드-테스트 매핑 | In Progress | `traceability_matrix_initial.md`, `docs/reviews/RV-030_p3_p4_traceability_alignment_review.md` |
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
| Integration Verification | cross-module scenario test | In Progress | `tests/integration/test_rsrx_session_supervisor_flow.c`, `integration_harness_test_spec_draft.md` |
| Safety Evidence | MISRA, static analysis, formal review records | In Progress | `docs/evidence/tooling/static_analysis_toolchain_baseline.md`, `docs/evidence/reports/static_analysis_report_2026-03-17_sa2_cppcheck.md`, `docs/reviews/RV-002_cppcheck_baseline_review.md`, `docs/reviews/RV-030_p3_p4_traceability_alignment_review.md` |

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
| M17 | integration harness 진입 | Completed | happy path, retransmission recovery, timeout fail-safe, channel-down fail-safe, decode failure, send failure budget, budget reset, bounded pump stability, bounded soak tests |
| M18 | channel manager wiring 진입 | Completed | session config wiring, validator gate, adapter failover selection test |
| M19 | supervisor failover runtime flow 도입 | Completed | channel-down failover decision, unit/integration failover tests |
| M20 | preferred recovery policy 도입 | Completed | channel manager auto-switch, adapter recovery selection, integration preferred recovery test |
| M21 | recovery holdoff policy 도입 | Completed | channel manager holdoff config, adapter holdoff selection, integration holdoff recovery test |
| M22 | channel switching telemetry 도입 | Completed | cumulative switch count, supervisor telemetry propagation, unit/integration telemetry assertions |
| M23 | redundancy flap soak integration 도입 | Completed | repeated failover/recovery cycle, holdoff reset verification, cumulative switch telemetry retention |
| M24 | supervisor decision telemetry refinement | Completed | decision class normalization, cumulative decision counter, unit telemetry assertions |
| M25 | supervisor channel-up refresh semantics 도입 | Completed | channel-up trigger refresh, preferred recovery via supervisor event path, unit coverage |
| M26 | static analysis evidence baseline 수립 | Completed | evidence index, static analysis plan, checklist, MISRA deviation log template |
| M27 | first static analysis baseline report 기록 | Completed | SA-REP-001, RV-001, checklist sign-off, deviation log update |
| M28 | static analysis toolchain baseline 확정 | Completed | gcc warning gate, cppcheck baseline, toolchain doc, reusable script, SA-REP-002 |
| M29 | static analysis follow-up cleanup 1차 | Completed | targeted redundant assignment cleanup, SA-REP-003, RV-003 |
| M30 | static analysis residual cleanup 완료 | Completed | targeted residual cleanup, SA-REP-004, RV-004 |
| M31 | wider static analysis baseline cleanup 완료 | Completed | broader style finding triage, SA-REP-005, RV-005 |
| M32 | outbound queue policy evidence baseline 수립 | Completed | `EVID-OUT-001`, `RV-006`, queue/backpressure policy rationale와 verification linkage |
| M33 | CI execution linkage baseline 수립 | Completed | `run_ci_verification.sh`, `sil4-ci.yml`, `EVID-CI-001`, `RV-007` |
| M34 | severity mapping baseline 수립 | Completed | `EVID-CI-003`, `RV-008`, compiler/cppcheck/diagnostic severity 기준 |
| M35 | CI artifact retention baseline 수립 | Completed | step summary, `sil4-ci-logs` artifact upload, CI output linkage |
| M36 | MISRA subset severity baseline 수립 | Completed | `EVID-CI-005`, `RV-009`, subset-level severity와 deviation linkage 준비 |
| M37 | tool-specific MISRA mapping baseline 수립 | Completed | `EVID-CI-007`, `RV-010`, compiler/cppcheck 출력과 subset mapping 연결 |
| M38 | second-tool candidate baseline 수립 | Completed | `EVID-CI-009`, `RV-011`, `clang` 우선 도입 후보와 선택 기준 고정 |
| M39 | clang first-run baseline 실행 완료 | Completed | `SA-REP-006`, `RV-012`, second-tool executable baseline 확보 |
| M40 | CI subset summary baseline 수립 | Completed | `run_ci_verification.sh`, `RV-013`, severity/subset bucket summary artifact 확보 |
| M41 | vendor rule ID mapping draft 수립 | Completed | `EVID-CI-012`, `RV-014`, future commercial analyzer 확장 구조 고정 |
| M42 | ctest registration strategy baseline 수립 | Completed | `EVID-CI-014`, `RV-015`, direct-run 유지와 ctest 전환 조건 고정 |
| M43 | PR annotation strategy baseline 수립 | Completed | `EVID-CI-016`, `RV-016`, severity/subset 기반 PR reporting policy 고정 |
| M44 | static analysis report template refinement 완료 | Completed | `EVID-SA-004`, `RV-017`, severity/subset/vendor-aware report section 확보 |
| M45 | vendor matrix sample baseline 수립 | Completed | `EVID-CI-018`, `RV-018`, exact vendor rule entry 예시 구조 확보 |
| M46 | PR annotation helper 구현 | Completed | `render_pr_annotation.sh`, `sil4-ci.yml`, step summary 연동 |
| M47 | first vendor deviation example 수립 | Completed | `EVID-CI-021`, `RV-020`, vendor-aware deviation workflow reference example 확보 |
| M48 | PR comment API linkage 구현 | Completed | `sil4-ci.yml`, sticky PR comment update 경로 확보 |
| M49 | delta-based annotation policy baseline 수립 | Completed | `EVID-CI-024`, `RV-022`, delta escalation/noise threshold 기준 고정 |
| M50 | delta-aware helper implementation 수립 | Completed | `render_pr_annotation.sh`, `RV-023`, optional baseline file 기준 delta 계산 구현 |
| M51 | first real vendor onboarding procedure 수립 | Completed | `EVID-CI-027`, `RV-024`, first actual vendor intake/deviation/report chain 기준 고정 |
| M52 | baseline persistence source policy 수립 | Completed | `EVID-CI-029`, `RV-025`, PR/branch baseline source priority와 fallback 기준 고정 |
| M53 | baseline artifact fetch implementation 수립 | Completed | `sil4-ci.yml`, `materialize_baseline_summary.sh`, `RV-026`, prior artifact fetch/materialize 경로 구현 |
| M54 | first actual vendor rule entry sample 수립 | Completed | `EVID-CI-032`, `RV-027`, first operational vendor entry/report/review sample 확보 |
| M55 | audit trail closeout baseline 수립 | Completed | `EVID-CI-034`, `RV-028`, 초기 evidence package 종료 기준 고정 |
| M56 | P3/P4 traceability and roadmap alignment review | Completed | `RV-030`, recent `TC-INT-017~039` traceability backfill, roadmap metadata/status 정합화 |

## In-Progress Items

| Item ID | Item | Current State | Exit Criteria |
| --- | --- | --- | --- |
| IP-001 | Public API hardening | timer ingress, outbound send, application delivery contract, bounded `outstanding 1 + deferred 1` queue, outbound telemetry, overflow reject API/diagnostic correlation과 integration coverage, repeated reject streak telemetry, threshold-based escalation, escalation hit telemetry 포함 | queueing/backpressure policy와 callback/report semantics 결정 |
| IP-002 | Codec maturation | deterministic skeleton과 outbound encode 연결 완료 | 실제 protocol field rules, length/range checks, negative vectors 보강 |
| IP-003 | Protocol context maturation | confirmation validation, invalid/regressing confirmation integration, initial zero sequence rejection integration, sequence gap detail, repeated gap retransmission/recovery integration, retransmission timeout fail-safe integration, failover recovery/timeout/repeated-gap recovery/repeated-gap timeout/stale rejection/unconfirmed-recovery rejection/repeated-gap unconfirmed-recovery rejection/invalid-confirmation/regressing-confirmation ordering integration, retransmission channel-up holdoff recovery/timeout integration, recovery success semantics, unconfirmed/stale retransmission rejection integration, retransmission confirm rules, duplicate inbound rejection 구현 | richer edge cases 보강 |
| IP-004 | Transport supervisor maturation | inbound decode handoff, sequence gate, poll receive, bounded pump loop, channel state gate, send result/timer delegation, outstanding-send correlation, channel-scoped send failure budget, receive error budget/reset/failover-carryover integration, send-failure budget failover-reset integration, preferred-recovery send-budget isolation integration, preferred-recovery receive-error carryover/reset integration, preferred-recovery mixed transient reset integration, holdoff send-budget isolation integration, holdoff stale feedback isolation/recovery-ordering/soak integration, holdoff receive-error carryover integration, holdoff mixed transient reset integration, holdoff flap transient asymmetry integration, mixed transient budget reset integration, channel-up refresh holdoff/flap-reset integration, failover transient recovery/soak integration, redundancy flap transient soak integration, redundancy flap transient long-run integration, channel-up holdoff transient soak integration, retransmission timeout/failover recovery/retransmission channel-up holdoff recovery/retransmission channel-up holdoff timeout/failover timeout/failover repeated-gap recovery/failover repeated-gap timeout/failover stale rejection/failover unconfirmed recovery rejection/failover repeated-gap unconfirmed recovery rejection/failover invalid confirmation/failover regressing confirmation integration, inactive-channel feedback filtering, decision telemetry, budget update/reset observability, deferred queue telemetry exposure 구현 및 integration 검증 진행 | integration-facing runtime event model과 channel-scoped send budget / non-channel-scoped receive budget semantics 반영 |
| IP-005 | Traceability enrichment | 최근 `TC-INT-017~039` integration 항목과 `RV-030` review linkage까지 backfill 완료 | 모든 구현 모듈과 테스트, 리뷰 항목 연결 |

## Not-Started Items

| Item ID | Item | Why It Matters | Planned Entry Point |
| --- | --- | --- | --- |
| NS-001 | Detailed Sequence Validation | inbound/outbound sequence gap 판단과 confirm 검증이 아직 단순화돼 있음 | retransmission confirm variants와 richer recovery/failure ordering 추가 |
| NS-002 | Transport Supervisor Completion | runtime feedback policy와 retry semantics 추가 필요 | runtime fault ordering과 queue/backpressure policy 확장 |
| NS-003 | Redundancy/Channel Manager | 실제 RaSTA 특성 대응 핵심 | richer hysteresis, flap suppression 세분화, switching audit 정책 확장 |
| NS-005 | Integration Test Harness Expansion | unit만으로는 안전 시나리오 커버 불가 | fake transport/fake time 기반 harness를 richer redundancy와 longer-run 시나리오로 확장 |
| NS-006 | Static Analysis and MISRA Evidence | SIL4 과제의 핵심 증빙 | first workflow baseline fetch success evidence와 first actual vendor evidence set 확보 |
| NS-007 | Review Records and Safety Evidence | 심사 대응 산출물 필요 | actual vendor finding review와 runtime CI evidence를 audit trail에 연결 |

## Readiness Assessment

| Area | Readiness | Comment |
| --- | --- | --- |
| 설계 구조 | High | 모듈 경계와 책임 분리는 많이 안정됨 |
| 단위 테스트 기반 | Medium | 핵심 skeleton coverage는 있으나 protocol complete 수준은 아님 |
| 프로토콜 완성도 | Medium | timer ingress, outbound encode, repeated gap/recovery 경로는 연결됐지만 richer sequencing과 redundancy는 미완 |
| 통합 가능성 | Medium-High | 주요 경계와 다수의 fault-integration path는 연결됐지만 long-run과 일부 ordering 규칙은 미완 |
| 인증 증빙 준비 | Medium | baseline policy/evidence는 많이 정리됐지만 actual runtime/vendor evidence는 아직 부족 |

## Current Risks

| Risk ID | Risk | Impact | Mitigation Direction |
| --- | --- | --- | --- |
| R-001 | detailed sequence validation 부분 미완 | repeated gap/recovery, failover stale/unconfirmed/repeated-gap unconfirmed recovery rejection, retransmission channel-up holdoff recovery/timeout은 반영됐지만 confirm ordering과 richer retransmission variants는 아직 단순화돼 있음 | protocol context와 supervisor 규칙 확장 |
| R-002 | transport supervisor 운영 루프 부분 미완 | outstanding-send correlation, channel-scoped budget, stale feedback filtering, holdoff stale feedback isolation/recovery ordering/soak, retransmission channel-up holdoff recovery/timeout ordering은 추가됐지만 runtime ordering과 retry policy는 아직 단순화돼 있음 | runtime feedback rule과 retry semantics 확장 |
| R-003 | redundancy policy 미완 | holdoff와 flap soak/refresh reset, flap transient soak, flap transient long-run soak, channel-up holdoff transient soak 검증은 있으나 richer hysteresis와 장시간 stability 규칙이 없다 | redundancy policy 세분화와 longer-run integration 확장 |
| R-004 | outbound application send가 minimal bounded queue 모델에 머묾 | `outstanding 1 + deferred 1`과 overflow telemetry는 들어갔지만 deeper queue/backpressure/retry semantics는 아직 단순하다 | queueing policy와 runtime feedback contract 설계 |
| R-005 | 인증 증빙 자동화 부족 | baseline policy와 review 묶음은 정리됐지만 actual CI runtime evidence와 actual vendor evidence set이 아직 없다 | first workflow baseline fetch success evidence 확보, first actual vendor evidence set 확보 |

## Recommended Next Order

1. `Transport Supervisor Maturation`
2. `Redundancy and Channel Manager`
3. `Integration Test Harness Expansion`
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
