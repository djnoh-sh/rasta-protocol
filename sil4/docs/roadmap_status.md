# SIL4 Reimplementation Roadmap Status

## Document Control

- Document ID: `PLAN-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-26`

## Summary

- 현재 전체 진행률 추정: `91~93%`
- 현재 단계: `P3/P4 representative closeout 이후 next policy growth 정리 + R-005 artifact-availability-only 단계`
- 다음 주력 단계: `actual CI/vendor evidence execution`, `redundancy next policy growth`, `sequencing/runtime next parity growth`

## Overall Phase Status

| Phase ID | Phase | Goal | Status | Progress Estimate | Notes |
| --- | --- | --- | --- | --- | --- |
| P1 | 계획/기준선 수립 | 규칙, 문서 구조, 요구사항/HLD/초기 추적성 확립 | Completed | 100% | 작업 규칙과 기본 산출물 체계 정착 |
| P2 | 코어 구조 설계/구현 | 상태 머신, orchestrator, API, abstraction, validator 뼈대 구현 | Completed | 100% | 주요 모듈 골격과 단위 테스트 확보 |
| P3 | 프로토콜 동작 구체화 | timer, sequencing, retransmission, outbound/inbound complete flow 구현 | In Progress | 91% | current P3는 protocol sequencing representative family 기준으로 closeout 상태다. ordering closeout matrix, protocol variant representative integration, repeated-gap/recovery/post-recovery ordering family, failover confirmation ordering family까지 정리돼 있다. 따라서 phase residual은 current sequencing inventory 추가가 아니라 `CONNECT_RESPONSE` 계열 parity, richer confirm/retransmission ordering variant, future message family growth, additional session-supervisor integration parity 같은 next sequencing policy growth 쪽에 더 가깝다 |
| P4 | 통합/강건성 검증 | integration harness, 장시간/경계/고장주입 시험 | In Progress | 96% | current P4는 integration/matrix representative family 기준으로 closeout 상태다. redundancy 쪽은 holdoff/flap-reset/active-loss-bypass/threshold `2..16` narrative, threshold-aware terminal outcome direct coverage `3..16`, switch-audit envelope, terminal outcome envelope, broader stability representative wrapper set까지 정리돼 있다. runtime/sequencing 쪽도 runtime ordering closeout, budget scope closeout, queue/backpressure closeout, ordering matrix family와 representative integration이 traceability/roadmap/review까지 연결된 상태다. 따라서 phase residual은 current family inventory 추가가 아니라 broader longer-run redundancy generalization, current envelope 밖의 switch-audit/runtime-fault policy growth, richer sequencing/runtime fault-ordering variant 쪽에 더 가깝다 |
| P5 | 인증 증빙 강화 | static analysis, MISRA evidence, review record, safety case 입력 생성 | In Progress | 94% | current P5는 evidence helper/tooling/execution-chain family 기준으로 representative closeout 상태다. operational packet/input/artifact runner chain, summary/receipt/bundle validation, verification phase ordering, helper ordering evidence, git index write ordering까지 문서와 helper 수준에서 정리돼 있다. 따라서 phase residual은 helper/tooling gap이 아니라 actual CI/vendor evidence acquisition, first workflow baseline fetch success evidence, first actual vendor evidence set execution 같은 artifact-availability/execution residual 쪽에 더 가깝다 |

## Workstream Status

| Workstream | Scope | Current Status | Evidence |
| --- | --- | --- | --- |
| Rules and Governance | 과제 운영 규칙, 코딩 규칙, 작업공간 기준 | Completed | `SIL4_REIMPLEMENTATION_RULES.md`, `CODING_RULES.md`, `sil4/README.md` |
| Requirements and HLD | SRS, hazard, architecture | In Progress | `system_requirements_draft.md`, `hazard_log_draft.md`, `reimplementation_architecture_draft.md` |
| Traceability | 요구사항-설계-코드-테스트 매핑 | In Progress | `traceability_matrix_initial.md`, `RV-030`, `RV-035`, `RV-036`, `RV-037`, `RV-065`, `RV-068`, `RV-072`, `RV-074`, `RV-080` 기준으로 current closeout wrapper와 residual scope alignment까지 연결 |
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
| M17 | integration harness 진입 | Completed | initial happy-path/fail-safe/budget-reset/bounded-pump representative integration family |
| M18 | channel manager wiring 진입 | Completed | session config wiring, validator gate, adapter failover selection test |
| M19 | supervisor failover runtime flow 도입 | Completed | channel-down failover decision, unit/integration failover tests |
| M20 | preferred recovery policy 도입 | Completed | channel manager auto-switch, adapter recovery selection, integration preferred recovery test |
| M21 | recovery holdoff policy 도입 | Completed | channel manager holdoff config, adapter holdoff selection, integration holdoff recovery test |
| M22 | channel switching telemetry 도입 | Completed | cumulative switch count, supervisor telemetry propagation, unit/integration telemetry assertions |
| M23 | redundancy flap soak integration 도입 | Completed | repeated failover/recovery soak, holdoff reset parity, cumulative switch telemetry retention |
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
| M50 | delta-aware helper implementation 수립 | Completed | `render_pr_annotation.sh`, `RV-023`, optional baseline delta calculation path |
| M51 | first real vendor onboarding procedure 수립 | Completed | `EVID-CI-027`, `RV-024`, first actual vendor onboarding/deviation/report chain baseline |
| M52 | baseline persistence source policy 수립 | Completed | `EVID-CI-029`, `RV-025`, PR/branch baseline source priority and fallback policy |
| M53 | baseline artifact fetch implementation 수립 | Completed | `sil4-ci.yml`, `materialize_baseline_summary.sh`, `RV-026`, prior artifact fetch/materialize path |
| M54 | first actual vendor rule entry sample 수립 | Completed | `EVID-CI-032`, `RV-027`, first operational vendor entry/report/review sample |
| M55 | audit trail closeout baseline 수립 | Completed | `EVID-CI-034`, `RV-028`, 초기 evidence package 종료 기준 고정 |
| M56 | P3/P4 traceability and roadmap alignment review | Completed | `RV-030`, recent integration traceability backfill, roadmap metadata/status alignment |

## In-Progress Items

| Item ID | Item | Current State | Exit Criteria |
| --- | --- | --- | --- |
| IP-001 | Public API hardening | current public API/bounded queue family는 representative closeout 상태다. timer ingress, outbound send, application delivery contract, bounded queue telemetry/escalation observability까지 정리돼 있다 | queueing/backpressure policy와 callback/report semantics 결정 |
| IP-002 | Codec maturation | deterministic skeleton과 outbound encode 연결 완료 | 실제 protocol field rules, length/range checks, negative vectors 보강 |
| IP-003 | Protocol context maturation | current protocol sequencing representative family는 closeout 상태다. ordering matrix, protocol variant representative integration, repeated-gap/recovery/post-recovery ordering family, failover confirmation ordering family까지 정리돼 있다 | richer confirm/retransmission edge cases와 future message family parity 보강 |
| IP-004 | Transport supervisor maturation | current runtime feedback/redundancy integration family는 representative closeout 상태다. runtime ordering, budget scope, holdoff/recovery/reset/feedback long-run family, telemetry observability, deferred queue exposure까지 정리돼 있다 | integration-facing runtime event model과 channel-scoped send budget / non-channel-scoped receive budget semantics 반영 |
| IP-005 | Traceability enrichment | current closeout wrapper와 residual-scope alignment backfill은 완료됐다. 최근 integration 항목과 주요 review linkage까지 current roadmap 기준으로 정리돼 있다 | 모든 구현 모듈과 테스트, 리뷰 항목 연결 |

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
| 설계 구조 | High | 모듈 경계와 책임 분리는 안정적이고 review/traceability alignment도 유지 중 |
| 단위 테스트 기반 | Medium-High | protocol context와 transport supervisor 핵심 규칙은 matrix와 closeout wrapper 수준까지 도달 |
| 프로토콜 완성도 | Medium-High | sequencing/retransmission/confirmation family는 representative closeout 상태이며 잔여는 residual variant 수준 |
| 통합 가능성 | Medium-High | 주요 fault-integration family는 연결됐고 잔여는 broader long-run/generalization 수준 |
| 인증 증빙 준비 | High | evidence helper/tooling/execution-chain family는 representative closeout 상태이며 잔여는 artifact-availability-only residual |

## Current Risks

| Risk ID | Risk | Impact | Mitigation Direction |
| --- | --- | --- | --- |
| R-001 | protocol sequencing의 다음 parity 단계가 아직 남아 있음 | current protocol sequencing family는 ordering closeout matrix와 protocol variant representative integration 기준으로 representative closeout 상태다. 따라서 residual은 broad sequencing 미완이 아니라 `CONNECT_RESPONSE` 계열 parity, future message family growth, additional session-supervisor integration parity 같은 next sequencing policy growth 쪽에 더 가깝다 | next protocol family parity 정리 |
| R-002 | transport supervisor runtime feedback의 다음 refinement 단계가 아직 남아 있음 | current runtime feedback family는 runtime ordering closeout, representative integration, budget scope closeout 기준으로 representative closeout 상태다. 따라서 residual은 broad runtime loop 미완이 아니라 queue growth 이후 retry/runtime feedback parity, 일부 correlated feedback refinement, `R-004`와의 경계 정리 같은 next runtime policy growth 쪽에 더 가깝다 | next runtime feedback parity 정리 |
| R-003 | redundancy policy의 다음 refinement 단계가 아직 남아 있음 | current active-standby/holdoff/bypass family는 representative closeout 상태다. holdoff threshold와 flap-reset representative coverage는 `2..16` narrative 기준으로 정리돼 있고, threshold-aware terminal outcome direct coverage도 `3..16`까지 닫혀 있다. current switch-audit family는 trigger/origin/holdoff-outcome/terminal-outcome-aware envelope 기준으로 representative closeout 상태이며, broader redundancy stability family도 long-run, non-terminal feedback, bypass re-entry, flap-bypass receive representative wrapper set으로 정리돼 roadmap/review/spec/traceability까지 explicit linkage를 가진다. 따라서 current residual은 current family 내부 representative parity gap이 아니라, threshold family generalization beyond direct terminal-outcome coverage `3..16`, broader `2..16` family narrative alignment 이후의 next policy growth, current switch-audit envelope 밖의 policy growth, broader long-run stability generalization, future redundancy mode growth 쪽에 더 가깝다 | next redundancy policy와 longer-run stability contract 정리 |
| R-004 | outbound application send의 다음 queue policy 단계가 아직 열려 있음 | current bounded `outstanding 1 + deferred 2` queue family는 queue/backpressure closeout matrix와 representative integration 기준으로 representative closeout 상태다. deferred queue telemetry/FIFO/mixed-clear/overflow-busy-reject/reset-source observability도 current model 기준으로 정리돼 있다. 따라서 residual은 current model 내부 coverage gap이 아니라 deeper backlog policy, current FIFO를 넘는 fairness policy, queue growth 이후 richer retry/runtime feedback semantics 같은 next queue policy growth 쪽에 더 가깝다 | next queueing policy와 runtime feedback contract 설계 |
| R-005 | 인증 증빙의 first operational artifact availability가 아직 없음 | current evidence helper/tooling/execution-chain family는 representative closeout 상태다. operational packet/input/artifact-dir runner chain, summary/receipt/bundle validation, readiness preflight/update/command/tracker-row/audit-note helper, one-shot actual execution starter, starter handoff summary helper, starter closeout bundle helper, starter patch guidance helper, actual close guidance helper, review/link guidance helper까지 execution guidance chain 기준으로 정리돼 있고 `RV-202`~`RV-214` recent helper bundle도 index/roadmap/review 기준으로 묶여 있다. residual도 artifact-availability-only로 고정돼 있으므로 current residual은 구조나 helper gap이 아니라 first workflow baseline fetch success artifact availability와 first actual vendor finding export artifact availability 부재 자체다 | first workflow baseline fetch success evidence 확보, first actual vendor evidence set execution |

## Recommended Next Order

1. `Actual CI/Vendor Evidence Acquisition`
2. `Redundancy Next Policy Growth`
3. `Protocol Sequencing Next Parity Growth`
4. `Transport Supervisor Runtime Feedback Growth`

## Next Gate Definition

- Gate Name: `G-P3-P4-Closeout`
- 목표:
  - `R-001~R-003`가 current representative closeout state 이후의 next policy-growth residual만 가리키도록 유지될 것
  - `P4`가 current family inventory가 아니라 current closeout state와 next growth area만 가리키도록 유지될 것
  - `R-005`는 계속 helper/tooling gap이 아니라 artifact-availability-only residual로 유지될 것
- 통과 조건:
  - `P3/P4` closeout wrapper와 representative integration이 traceability/roadmap/review까지 연결될 것
  - build, unit/integration test, `cppcheck`가 깨끗할 것
  - roadmap의 `R-001~R-005` 설명이 current closeout state와 실제 남은 큰 작업만 가리키도록 정리될 것
