# SIL4 Reimplementation Roadmap Status

## Document Control

- Document ID: `PLAN-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-01`

## Summary

- 현재 전체 진행률 추정: `93~95%`
- 현재 상태: `P3/P4 representative closeout` 기준선은 유지 중이며, residual은 구현 누락보다 next policy/security/evidence growth에 집중돼 있다.
- 최근 업데이트: `TC-INT-210`/`RV-418`로 option A redundancy-carried no-checksum SR runtime을 fake transport + real codec + supervisor/session handoff 통합 경계까지 확장했다.
- 다음 주력 단계: `actual CI/vendor evidence execution`, `AM263Px/SafeRTOS target artifact acquisition`, `redundancy next policy growth`, `protocol sequencing next parity growth`, `non-none checksum algorithm implementation if selected`
- 상세 변경 이력은 `docs/reviews/RV-*`, `docs/verification/*_spec*_draft.md`, `docs/evidence/**`, `vv_reports/**`를 기준 증거로 삼는다.

## Overall Phase Status

| Phase ID | Phase | Status | Progress | Current Meaning |
| --- | --- | --- | --- | --- |
| P1 | 계획/기준선 수립 | Completed | 100% | 운영 규칙, 문서 구조, 요구사항/HLD/초기 추적성 체계 정착 |
| P2 | 코어 구조 설계/구현 | Completed | 100% | 상태 머신, orchestrator, API, abstraction, validator 골격과 단위 테스트 확보 |
| P3 | 프로토콜 동작 구체화 | In Progress | 92% | sequencing/retransmission/confirmation representative family는 closeout 상태이며 residual은 richer ordering/message-family growth |
| P4 | 통합/강건성 검증 | In Progress | 96% | redundancy, runtime feedback, queue/backpressure representative matrix는 closeout 상태이며 residual은 next semantic policy growth |
| P5 | 인증 증빙 강화 | In Progress | 95% | helper/tooling/execution-chain은 준비됐고 residual은 actual vendor/target-qualified artifact availability |

## Workstream Status

| Workstream | Status | Current Evidence | Residual |
| --- | --- | --- | --- |
| Rules and Governance | Completed | `SIL4_REIMPLEMENTATION_RULES.md`, `CODING_RULES.md`, `sil4/README.md` | 유지 관리 |
| Requirements and HLD | In Progress | `system_requirements_draft.md`, `hazard_log_draft.md`, `reimplementation_architecture_draft.md` | 인증/타깃 요구사항 정제 |
| Traceability | In Progress | `traceability_matrix_initial.md`, `RV-030`, `RV-217..RV-326`, `RV-332..RV-418` | 새 policy/evidence growth마다 review/spec linkage 유지 |
| State Machine / Orchestrator | In Progress | `rsrx_state_machine.*`, `rsrx_orchestrator.*`, unit tests, reset baseline evidence `RV-393..RV-394` | future state/action 확장 시 assertion density 유지 |
| Platform / Transport Abstraction | Completed | `rsrx_platform.h`, `rsrx_transport.h`, contract tests | 포팅 시 target adapter evidence |
| Adapter Layer / Public API | In Progress | `rsrx_platform_adapters.*`, `rsrx_api.*`, `TC-PA-010..011`, `TC-API-014..018` | target runtime binding, selected codec policy 유지 |
| Application Data Contract | Completed | application data LLD/spec, API/adapter tests | integration expansion only |
| Protocol Context | In Progress | `TC-PC-004`, `TC-PC-019..028`, sequence/base/confirmation/API/recovery-cleanup/init-baseline guards | richer confirm/retransmission variants |
| Transport Supervisor | In Progress | `TC-SUP-001..077`, `TC-INT-210`, runtime/channel/codec-status/SR-runtime/identity/redundancy-SR reports, public API guard evidence `RV-380`, init switch/runtime-loop baseline evidence `RV-388..RV-389`, SR runtime wiring evidence `RV-408`, SR identity wiring evidence `RV-410`, redundancy-carried SR runtime wiring/integration evidence `RV-417..RV-418` | richer runtime fault-ordering variants |
| Queue / Backpressure | In Progress | bounded `outstanding 1 + deferred 12`, queue/fairness/long-run matrices | future fairness/retry/runtime-feedback semantics |
| Redundancy / Channel Manager | In Progress | holdoff `2..20`, flap-reset `2..19`, terminal outcome `3..19`, topology validation, unsupported `REDUNDANT` topology rejection, public API guard evidence `TC-CHM-059`/`RV-379`, flap-penalty saturation evidence `TC-CHM-060`/`RV-381`, reset audit preservation `TC-CHM-061`/`RV-387` | future redundancy mode and longer-run policy growth |
| Codec / Security | In Progress | `TC-CODEC-001..050`, `TC-SUP-075..077`, `TC-INT-206`, `TC-INT-208`, `TC-INT-210`, `RV-337`, `RV-354..RV-367`, `RV-369..RV-378`, `RV-382..RV-384`, `RV-390`, `INV-RASTA-001`, `PDU-PARITY-001A..001F`, `RED-PDU-PARITY-001A`, `RV-399..RV-418` | non-none checksum algorithm implementation if selected, CRC-bearing redundancy PDU behavior if selected, optional MAC/security extension taxonomy, vendor security vectors |
| Configuration Validation | Completed | `TC-CFG-003`, `TC-CFG-008..010` | deployment-specific policy additions |
| Integration Verification | In Progress | `test_rsrx_session_supervisor_flow.c`, integration harness spec, `TC-INT-205..210` | target/longer-run integration expansion |
| Safety Evidence | In Progress | cppcheck reports, stack/memory runbook/helper, strict-warning policy, operational evidence snapshot, V&V v1.6 official response, `EVID-TGT-001` | actual vendor finding export and target-qualified stack/memory-map artifacts |
| AM263Px / SafeRTOS Porting | Planned | `EVID-TGT-001`, `RV-415`; portable core/adapter boundary is ready; target-specific layer intentionally not implemented in core | SafeRTOS task/timer/queue/critical-section binding, TI driver transport adapter, optional hardware-backed CRC/crypto adapter, target stack/memory/timing evidence |

## Current Evidence Baseline

- Verification baseline remains `build -> unit/integration tests -> cppcheck`.
- Current host verification command set:
  - `cmake -S sil4 -B /tmp/sil4-build`
  - `cmake --build /tmp/sil4-build -j4`
  - all `/tmp/sil4-build/rsrx_*_test` unit/integration executables
  - `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
- Latest local verification was green after `RV-418`.
- Latest document-only planning update was `RV-415`; no host verification rerun was required for that document-only step.
- External artifact references:
  - closed baseline fetch artifact: `sil4-ci-logs2/*`, source run `24661353609`, fetch run `24662424670`
  - remaining vendor evidence artifact: raw vendor export or secured attachment reference, `vendor_export_context.env`, vendor rule/file/location metadata, capture or workflow run page
  - remaining stack/memory artifact: target-release or vendor-qualified stack-bound/static memory-map package, not the host helper output
  - remaining AM263Px/SafeRTOS artifact: target porting package with SafeRTOS task/queue/timer policy, TI driver transport binding evidence, optional hardware CRC/crypto adapter evidence, software-vs-hardware CRC equivalence logs, hardware self-test/timeout/diagnostic evidence, linker map, stack usage, timing/WCET evidence, target integration logs

## Current Risks

| Risk ID | Current Baseline | Remaining Residual | Next Action |
| --- | --- | --- | --- |
| R-001 Protocol sequencing | Current inbound family, ordering representatives, init baseline, wrap guards, invalid type/confirmation/API guards, recovery cleanup state isolation, and unsequenced isolation are representative-closeout. | richer confirm/retransmission ordering variants, broader future message families, additional session-supervisor parity | next protocol family parity |
| R-002 Runtime feedback | Runtime ordering, init report/runtime-loop baseline, public API guard rejection, budget scope, receive-error stage/status, topology-mismatch budget integration, and escalation telemetry are representative-closeout. | richer runtime-fault variants and queue-growth semantics | next runtime feedback policy |
| R-003 Redundancy | Holdoff/flap/terminal parity, topology validation, unsupported `REDUNDANT` topology rejection, public API guard rejection, switch audit, reset audit preservation, penalty telemetry including saturation boundary, and stability wrappers are representative-closeout. | future redundancy mode growth and longer-run policy generalization | next redundancy policy |
| R-004 Queue policy | Current `outstanding 1 + deferred 12` depth is closed across adapter, supervisor, integration, and API overflow paths. | future fairness, retry, runtime-feedback, or configured-capacity semantics | next queueing policy |
| R-005 Evidence artifacts | Helper/tooling/runbooks, host stack/memory baseline, strict-warning policy, and baseline fetch artifact are prepared/closed as applicable. | first actual vendor finding export and target/vendor-qualified stack/memory-map evidence | vendor/target evidence execution |
| R-006 Codec/security | Current codec skeleton, CRC32 optional path, typed status taxonomy, selected CRC integration, calculator injection seam, policy gates, encode/decode stale-output representative evidence, decode null-argument output clear, CRC32 truncated-output clear, CRC32 inner status preservation including transport metadata misuse, reserved-header tamper output clear, unsupported-message/reason output clear, trailing/truncated/oversized payload output clear, direct misuse event/channel output clear, `INV-RASTA-001` inventory, `PDU-PARITY-001` SR wire-profile draft, profile/packet/mapping contracts, fixed big-endian SR byte-order helpers, no-checksum SR common-header/payload encode/decode, selected no-checksum default profile, checksum profile unsupported-rejection boundary, codec-level timestamp/window admission boundary, timestamp-admitted handoff mapping, supervisor runtime SR selection, receiver/sender identity admission boundary, supervisor identity policy wiring, redundancy PDU metadata profile, redundancy CRC option admission boundary, option A no-CRC redundancy PDU encode/decode behavior, carried no-checksum SR decode bridge, supervisor redundancy-carried SR runtime selection, and redundancy-carried SR integration handoff are in place. | non-none selected checksum algorithm implementation only if controlled requirement selects it, CRC-bearing redundancy PDU behavior if selected, optional MAC/security extension definition, additional tamper taxonomy, vendor-oriented security negative vectors | keep selected no-checksum/CRC-option-A baselines; implement CRC-bearing behavior only if selected |
| R-007 V&V accepted follow-up | Handshake action assertion density, `CONNECT_REQUEST` unsequenced baseline, and CMake warning-hardening follow-up are closed. | future maintenance only | keep assertion/warning discipline |
| R-008 AM263Px/SafeRTOS porting | Core protocol remains portable C with platform/transport/codec adapter boundaries; no AM263Px or SafeRTOS dependency is allowed in the core implementation. `EVID-TGT-001` defines the SafeRTOS task/timer/queue, TI driver transport, hardware CRC/crypto adapter, and target evidence checklist. Hardware-backed CRC/crypto acceleration may be used only behind target-specific codec/security adapters with portable software fallback retained. | actual target porting layer, SafeRTOS safety-manual compliance mapping, TI driver binding, optional hardware CRC/crypto adapter, software-vs-hardware equivalence evidence, HW self-test/timeout/diagnostic evidence, target stack/memory/timing evidence, target integration/fault-injection logs | acquire first target artifact package |

## Waiting / Backlog Items

| Item | Status | Blocker |
| --- | --- | --- |
| Vendor finding export evidence | Waiting | vendor-qualified tool/license/export access |
| Target-qualified stack/memory-map evidence | Waiting | target-release build/toolchain and qualified analyzer output |
| AM263Px/SafeRTOS porting layer | Planned | target artifact execution pending; planning artifact is `EVID-TGT-001` |
| AM263Px/SafeRTOS target evidence | Waiting | target hardware/build environment, SafeRTOS safety evidence, linker/stack/timing capture process, software-vs-hardware CRC equivalence and HW diagnostic capture process |
| RaSTA PDU/checksum/timestamp parity | Started | exact official/customer RaSTA clause mapping, non-none checksum algorithm implementation only if selected |
| MAC/security extension | Not Started | controlled requirement needed; otherwise keep `uRequireMac` as explicit unsupported extension gate |
| Future redundancy routing modes | Not Started | policy decision beyond current active-standby baseline |

## Readiness Assessment

| Area | Readiness | Comment |
| --- | --- | --- |
| 설계 구조 | High | 모듈 경계와 책임 분리는 안정적 |
| 단위 테스트 기반 | Medium-High | 핵심 protocol/supervisor/codec 규칙은 representative matrix 수준 |
| 프로토콜 완성도 | Medium-High | 잔여는 broad incompleteness가 아니라 next policy variants |
| 통합 가능성 | Medium-High | 주요 fault-integration family는 연결됨 |
| 인증 증빙 준비 | High | helper/tooling은 준비됐고 실제 vendor/target artifact가 남음 |

## Recommended Next Order

1. `Actual CI/Vendor Evidence Acquisition`
2. `AM263Px/SafeRTOS Target Artifact Acquisition`
3. `Redundancy Next Policy Growth`
4. `Protocol Sequencing Next Parity Growth`
5. `Non-None Checksum Algorithm Implementation If Selected`
6. `Transport Supervisor Runtime Feedback Growth`

## Next Gate Definition

- Gate Name: `G-P3-P4-Closeout`
- Gate intent: current representative closeout state를 유지하고, residual을 실제 next policy/security/evidence growth로만 관리한다.
- 통과 조건:
  - `R-001..R-008`이 current closeout state와 실제 residual만 가리킬 것
  - accepted V&V follow-up이 baseline defect wording이 아니라 explicit backlog/evidence item으로 유지될 것
  - AM263Px/SafeRTOS work가 core implementation dependency가 아니라 target porting/evidence workstream으로 유지되고, hardware acceleration은 target adapter 뒤에만 위치할 것
  - numeric parity growth는 current target range에서 closeout으로 유지하고, 새 policy/failure-mode가 있을 때만 확장할 것
  - build, unit/integration test, `cppcheck`가 깨끗할 것
