# SIL4 Reimplementation Roadmap Status

## Document Control

- Document ID: `PLAN-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Summary

- 현재 전체 진행률 추정: `92~94%`
- 현재 상태: `P3/P4 representative closeout` 기준선은 대체로 유지 중이며, 2026-06-02 comprehensive V&V audit의 public API 비동기 임계영역은 portable host guard 적용까지 진행됐고, 신규 Finding G는 codec timestamp admission corrective action으로 수용했다.
- 최근 업데이트: `RV-546`로 RaSTA SCI/application-message aggregation scope decision을 `Codec / Security` 및 `R-006` residual에도 반영했다.
- 다음 주력 단계: external lane은 `actual vendor export acquisition` 및 `first AM263Px/SafeRTOS target package execution`; local lane은 target context가 들어오면 `EVID-TGT-003`/`EVID-TGT-005`/`EVID-TGT-007` fill-in 지원, 그 외 checksum/security, protocol/redundancy policy, 새 wrapper status growth는 controlled requirement/status family 추가 시에만 진행
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
| Traceability | In Progress | `sil4/docs/traceability/traceability_matrix_initial.md`, `RV-030`, `RV-217..RV-326`, `RV-332..RV-546` | 새 policy/evidence growth마다 review/spec linkage 유지 |
| State Machine / Orchestrator | In Progress | `rsrx_state_machine.*`, `rsrx_orchestrator.*`, unit tests, reset baseline evidence `RV-393..RV-394` | future state/action 확장 시 assertion density 유지 |
| Platform / Transport Abstraction | In Progress | `rsrx_platform.h`, `rsrx_transport.h`, contract tests, critical-section port contract `TC-PLAT-004`/`RV-433`, portable API boundary closeout `RV-445` | 포팅 시 target adapter evidence |
| Adapter Layer / Public API | In Progress | `rsrx_platform_adapters.*`, `rsrx_api.*`, `TC-PA-010..015`, `TC-API-003`, `TC-API-014..029`, public API report-output guard evidence `RV-427`, adapter query/receive/executor output guard evidence `RV-429..RV-431`, reset timer quiescence evidence `RV-432`, critical-section startup gate evidence `TC-CFG-011`/`RV-433`, public API critical-section guard evidence `RV-434`, outbound telemetry snapshot evidence `RV-435`, outbound queue snapshot/report-helper/feedback-clear evidence `RV-436`/`RV-439`/`RV-442`, channel-manager snapshot evidence `RV-437`, inbound event resolve/record boundary evidence `RV-440`/`RV-441`, channel query/frame receive boundary evidence `RV-443`/`RV-444`, transport API balanced matrix evidence `RV-447` | target runtime binding, callback reentrancy policy, selected codec policy 유지 |
| Application Data Contract | Completed | application data LLD/spec, API/adapter tests | integration expansion only |
| Protocol Context | In Progress | `TC-PC-004`, `TC-PC-019..033`, `TC-INT-212..217`, sequence/base/confirmation/API/recovery-cleanup/init-baseline/stale-output/payload-pointer/record-order/gap-record/resolve-failure-output/retransmission-record/stale-retransmission-record/invalid-confirmation recovery-record guards, unsequenced disconnect/connect-request/diagnostic integration pass-through evidence `RV-504`/`RV-505`/`RV-510`/`RV-511`/`RV-513`/`RV-514`, repeated-gap request-state/cleanup evidence `RV-488`/`RV-489`, `RV-446`/`RV-448`/`RV-449` | richer confirm/retransmission variants |
| Transport Supervisor | In Progress | `TC-SUP-001..088`, `TC-INT-210..211`, runtime/channel/codec-status/SR-runtime/identity/redundancy-SR reports, budget scope evidence `RV-461`, send feedback ordering evidence `RV-462`, channel event ordering evidence `RV-463`, timer delegation evidence `RV-464`, switch audit closeout evidence `RV-465`, busy reject telemetry evidence `RV-466`, public API guard evidence `RV-380`/`RV-425`, init switch/runtime-loop baseline evidence `RV-388..RV-389`, SR runtime wiring evidence `RV-408`, SR identity wiring evidence `RV-410`, redundancy-carried SR runtime wiring/integration and oversized-status propagation evidence `RV-417..RV-418`/`RV-501`/`RV-502`, outbound queue snapshot refresh/helper/feedback-clear evidence `RV-436`/`RV-439`/`RV-442`, channel-manager snapshot refresh evidence `RV-437`, active-channel snapshot helper evidence `RV-438`, inbound event resolve/record boundary evidence `RV-440`/`RV-441`, channel query/frame receive boundary evidence `RV-443`/`RV-444`, frame receive telemetry evidence `RV-450`/`RV-452`/`RV-453`, retry-ordering boundary telemetry evidence `RV-454`, pump terminal/error/ignored/escalation/max-poll boundary telemetry evidence `RV-455`/`RV-456`/`RV-457`/`RV-458`/`RV-459`, runtime ordering closeout evidence `RV-460`, channel query telemetry evidence `RV-451`, terminal outcome unit/integration parity through threshold `20` evidence `RV-491..RV-493` | richer runtime fault-ordering variants |
| Queue / Backpressure | In Progress | bounded `outstanding 1 + deferred 12`, queue/fairness/long-run matrices | future fairness/retry/runtime-feedback semantics |
| Redundancy / Channel Manager | In Progress | holdoff `2..20`, flap-reset `2..20`, terminal outcome unit/integration `3..20`, topology validation, unsupported `REDUNDANT` topology rejection, public API guard evidence `TC-CHM-059`/`RV-379`/`RV-426`, flap-penalty saturation evidence `TC-CHM-060`/`RV-381`, reset audit preservation `TC-CHM-061`/`RV-387` | future redundancy mode and longer-run policy growth |
| Codec / Security | In Progress | `TC-CODEC-001..063`, `TC-SUP-075..077`, `TC-INT-206`, `TC-INT-208`, `TC-INT-210`, `RV-337`, `RV-354..RV-367`, `RV-369..RV-378`, `RV-382..RV-384`, `RV-390`, `INV-RASTA-001`, `PDU-PARITY-001A..001F`, `RED-PDU-PARITY-001A`, `RV-399..RV-418`, confirmed timestamp stale/causal-order guard and handoff evidence `RV-477`/`RV-478`/`RV-479`/`RV-494`, timestamp-admitted handoff oversized-payload evidence `RV-495`, identity+timestamp handoff reject propagation evidence `RV-496`, redundancy-carried SR outer malformed/oversized and inner truncated/trailing status-preservation/propagation evidence `RV-497`/`RV-498`/`RV-499`/`RV-500`/`RV-501`/`RV-502`, SR no-checksum encode/decode guard, payload-boundary, supported control-family, control wire-type byte, hand-authored control fixture decode, payload-bearing data fixture decode, and selected no-checksum parity reassessment evidence `RV-506`/`RV-507`/`RV-508`/`RV-509`/`RV-512`/`RV-515`/`RV-516`/`RV-517`/`RV-518`, CRC32 unsupported-message/reason, trailing-bytes, truncated-payload, oversized-payload, reserved-header, and transport-metadata inner status preservation matrix closeout evidence `RV-524`/`RV-525`/`RV-526`/`RV-527`/`RV-528`/`RV-529`, post-closeout priority reassessment evidence `RV-503`, unsupported numeric mapping clear evidence `RV-480`/`RV-481`/`RV-482`, identity handoff unsupported/null-policy/null-packet clear evidence `RV-483`/`RV-484`/`RV-486`/`RV-487`, timestamp handoff null-policy clear evidence `RV-485` | non-none checksum algorithm implementation if selected, CRC-bearing redundancy PDU behavior if selected, SCI/application aggregation if scoped in, optional MAC/security extension taxonomy, vendor security vectors |
| Configuration Validation | In Progress | `TC-CFG-003`, `TC-CFG-006`, `TC-CFG-008..011`, report/null-output guard evidence `RV-428`, critical-section gate evidence `RV-433` | deployment-specific policy additions |
| Integration Verification | In Progress | `test_rsrx_session_supervisor_flow.c`, integration harness spec, protocol/runtime/redundancy closeout evidence `RV-467`/`RV-468`/`RV-469`/`RV-470`/`RV-471`/`RV-472`/`RV-473`/`RV-474`/`RV-475`/`RV-476`, unsequenced disconnect/connect-request/diagnostic integration pass-through evidence `TC-INT-212..217`/`RV-504..RV-505`/`RV-510..RV-511`/`RV-513`/`RV-514`, terminal outcome threshold `20` integration evidence `RV-493`, `TC-INT-205..211` | target/longer-run integration expansion |
| Safety Evidence | In Progress | cppcheck reports, stack/memory runbook/helper, strict-warning policy, operational evidence snapshots `EVID-CI-108`/`EVID-CI-120`, V&V v1.6 official response, comprehensive V&V 2026-06-02 response including Finding G corrective action, `EVID-TGT-001`, `EVID-TGT-003`, `EVID-TGT-005`, `EVID-TGT-007`, `EVID-TGT-009`, portable API concurrency/timer-quiescence closeout `RV-445` | actual vendor finding export, target-qualified stack/memory-map artifacts, target concurrency evidence |
| AM263Px / SafeRTOS Porting | Planned | `EVID-TGT-001`, `EVID-TGT-003`, `EVID-TGT-005`, `EVID-TGT-007`, `EVID-TGT-009`, `RV-415`, `RV-519`, `RV-521`, `RV-522`, `RV-523`; portable core/adapter boundary is ready; target-specific layer intentionally not implemented in core | completed SafeRTOS binding artifact, completed callback policy artifact, TI driver transport adapter, optional hardware-backed CRC/crypto adapter, target stack/memory/timing evidence |

## Current Evidence Baseline

- Verification baseline remains `build -> unit/integration tests -> cppcheck`.
- Current host verification command set:
  - `cmake -S sil4 -B /tmp/sil4-build`
  - `cmake --build /tmp/sil4-build -j4`
  - all `/tmp/sil4-build/rsrx_*_test` unit/integration executables
  - `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
- Latest executable host verification baseline was green after `RV-528`.
- Changes from `RV-529` through `RV-546` are document-only closeout, wording, priority, or roadmap-evidence clarifications unless explicitly noted otherwise; they did not change source or executable test logic.
- Latest document-only RaSTA residual alignment is `RV-546`, which mirrors the SCI/application-message aggregation scope decision into `Codec / Security` and `R-006`; no host verification rerun was required for that document-only step.
- Latest document-only RaSTA scope clarification is `RV-545`, which keeps SCI/application-message aggregation outside the current PDU wire-profile claim until a controlled scope decision selects it; no host verification rerun was required for that document-only step.
- Latest document-only traceability path cleanup is `RV-544`, which aligns the roadmap evidence path with `sil4/docs/traceability/traceability_matrix_initial.md`; no host verification rerun was required for that document-only step.
- Latest document-only queue spec wording cleanup is `RV-542`, which aligns old `deferred 3` representative-path wording with the current `outstanding 1 + deferred 12` queue policy; no host verification rerun was required for that document-only step.
- Latest document-only roadmap readability cleanup is `RV-541`, which compresses the `RV-531..RV-540` RaSTA documentation alignment pass into the `RV-540` closeout anchor; no host verification rerun was required for that document-only step.
- Latest RaSTA documentation alignment closeout is `RV-540`, supported by `RV-531..RV-539`; further RaSTA wording changes are gated on new requirement/clause/target/V&V input.
- Latest document-only work-priority reassessment is `RV-530`, which confirms no additional unselected host-only implementation growth should proceed without external/target artifacts or a controlled requirement/status/policy selection; no host verification rerun was required for that document-only step.
- Latest document-only codec closeout update is `RV-529`, which closes the current CRC32 wrapper inner-status preservation matrix; no host verification rerun was required for that document-only step.
- Latest document-only planning update is `RV-523`, which adds the AM263Px/SafeRTOS target artifact package fill-in runbook; no host verification rerun was required for that document-only step.
- External artifact references:
  - closed baseline fetch artifact: `sil4-ci-logs2/*`, source run `24661353609`, fetch run `24662424670`
  - remaining vendor evidence artifact: raw vendor export or secured attachment reference, `vendor_export_context.env`, vendor rule/file/location metadata, capture or workflow run page
  - remaining stack/memory artifact: target-release or vendor-qualified stack-bound/static memory-map package, not the host helper output
  - remaining AM263Px/SafeRTOS artifact: target porting package with SafeRTOS task/queue/timer policy, TI driver transport binding evidence, optional hardware CRC/crypto adapter evidence, software-vs-hardware CRC equivalence logs, hardware self-test/timeout/diagnostic evidence, linker map, stack usage, timing/WCET evidence, target integration logs

## Current Risks

| Risk ID | Current Baseline | Remaining Residual | Next Action |
| --- | --- | --- | --- |
| R-001 Protocol sequencing | Current inbound family, ordering representatives, protocol ordering closeout traceability `RV-476`, protocol variant closeout traceability `RV-474`, post-recovery retransmission-request traceability `RV-475`, init baseline, wrap guards, invalid type/confirmation/API guards, encode failure stale-output clear, resolve failure stale-event clear, outbound payload pointer guard, inbound stale/gap/zero record guard, retransmission pending direct-record guard `RV-446`/`RV-448`/`RV-449`, unsequenced disconnect/connect-request/diagnostic integration pass-through evidence `RV-504`/`RV-505`/`RV-510`/`RV-511`/`RV-513`/`RV-514`, repeated-gap retransmission request state/cleanup evidence `RV-488`/`RV-489`, adapter rejected-record side-effect guard, recovery cleanup state isolation, and unsequenced isolation are representative-closeout. | richer confirm/retransmission ordering variants, broader future message families, additional session-supervisor parity | next protocol family parity |
| R-002 Runtime feedback | Runtime ordering, init report/runtime-loop baseline, supervisor/session public API guard rejection with stale report clear, budget scope integration traceability `RV-473`, receive-error stage/status, frame receive boundary telemetry, channel query telemetry, topology-mismatch budget integration, and escalation telemetry are representative-closeout. | richer runtime-fault variants and queue-growth semantics | next runtime feedback policy |
| R-003 Redundancy | Holdoff/flap parity through configured target `20`, terminal outcome unit/integration parity through `20`, topology validation, unsupported `REDUNDANT` topology rejection, public API guard rejection with stale selection result clear, switch audit, reset audit preservation, penalty telemetry including saturation boundary, stability wrappers, hysteresis integration traceability `RV-470`, long-run integration traceability `RV-471`, and flap-bypass closeout wrapper alignment `RV-472` are representative-closeout. | future redundancy mode growth and target-specific soak generalization | next redundancy policy |
| R-004 Queue policy | Current `outstanding 1 + deferred 12` depth is closed across adapter, supervisor, integration, and API overflow paths. | future fairness, retry, runtime-feedback, or configured-capacity semantics | next queueing policy |
| R-005 Evidence artifacts | Helper/tooling/runbooks, host stack/memory baseline, strict-warning policy, baseline fetch artifact, and updated readiness snapshot `EVID-CI-120` are prepared/closed as applicable. | first actual vendor finding export and target/vendor-qualified stack/memory-map evidence | acquire vendor export containing `vendor_export_context.env`; then rerun operational packet path |
| R-006 Codec/security | Current codec skeleton, CRC32 optional path, typed status taxonomy, selected CRC integration, calculator injection seam, policy gates, encode/decode stale-output representative evidence, decode null-argument output clear, CRC32 truncated-output clear, current CRC32 inner status preservation matrix representative-closeout `RV-529`, unsupported-message/reason output clear including out-of-range disconnect reason mapping, trailing/truncated/oversized payload output clear, direct misuse event/channel output clear, `INV-RASTA-001` inventory, `PDU-PARITY-001` SR wire-profile draft, profile/packet/mapping contracts, fixed big-endian SR byte-order helpers, no-checksum SR common-header/payload encode/decode including encode/decode guard, payload-boundary, supported control-family, control wire-type byte, hand-authored control fixture decode, payload-bearing data fixture decode, and selected no-checksum parity reassessment evidence `RV-506`/`RV-507`/`RV-508`/`RV-509`/`RV-512`/`RV-515`/`RV-516`/`RV-517`/`RV-518`, selected no-checksum default profile, checksum profile unsupported-rejection boundary, unsupported `INVALID`/`RetrResp`/`RetrData` numeric mapping clear behavior, codec-level timestamp/window admission boundary including confirmed timestamp stale and causal-order guards `RV-477`/`RV-494`, confirmed timestamp handoff clear evidence `RV-478`/`RV-485`, timestamp-admitted handoff bounded-payload evidence `RV-495`, identity+timestamp handoff reject propagation evidence `RV-496`, identity+timestamp handoff clear evidence `RV-479`/`RV-483`/`RV-484`/`RV-486`/`RV-487`, timestamp-admitted handoff mapping, supervisor runtime SR selection, receiver/sender identity admission boundary, supervisor identity policy wiring, redundancy PDU metadata profile, redundancy CRC option admission boundary, option A no-CRC redundancy PDU encode/decode behavior, carried no-checksum SR decode bridge with outer malformed/oversized and inner truncated/trailing status preservation plus supervisor/integration propagation `RV-497`/`RV-498`/`RV-499`/`RV-500`/`RV-501`/`RV-502`, supervisor redundancy-carried SR runtime selection, redundancy-carried SR integration handoff, and post-closeout priority reassessment `RV-503` are in place. | non-none selected checksum algorithm implementation only if controlled requirement selects it, CRC-bearing redundancy PDU behavior if selected, SCI/application-message aggregation if scoped in, optional MAC/security extension definition, additional tamper taxonomy only when a new status/security profile appears | target/vendor evidence or selected security-policy implementation |
| R-007 V&V accepted follow-up | Handshake action assertion density, `CONNECT_REQUEST` unsequenced baseline, and CMake warning-hardening follow-up are closed. | future maintenance only | keep assertion/warning discipline |
| R-008 AM263Px/SafeRTOS porting | Core protocol remains portable C with platform/transport/codec adapter boundaries; no AM263Px or SafeRTOS dependency is allowed in the core implementation. `EVID-TGT-001` defines the SafeRTOS task/timer/queue, TI driver transport, hardware CRC/crypto adapter, and target evidence checklist; `EVID-TGT-003` defines critical-section binding capture criteria; `EVID-TGT-005` defines the first target package landing structure; `EVID-TGT-007` defines callback reentrancy/deferred-callback capture criteria; `EVID-TGT-009` defines target package fill-in execution order. Hardware-backed CRC/crypto acceleration may be used only behind target-specific codec/security adapters with portable software fallback retained. | actual target porting layer, SafeRTOS safety-manual compliance mapping, TI driver binding, optional hardware CRC/crypto adapter, software-vs-hardware equivalence evidence, HW self-test/timeout/diagnostic evidence, target stack/memory/timing evidence, target integration/fault-injection logs | execute `EVID-TGT-009` when target environment is available |
| R-009 API concurrency and reset quiescence | Portable host scope is closed by `TC-API-019`/`RV-432` reset timer quiescence plus `RV-433..RV-447` critical-section/startup/snapshot/supervisor-boundary/balanced-matrix evidence. `RV-445` records that supervisor paths no longer directly call session transport/protocol/channel-manager internals for the audited portable boundary; they use session public API/snapshot boundaries. `EVID-TGT-003` defines target critical-section binding evidence and `EVID-TGT-007` defines callback reentrancy/deferred-callback policy evidence. | completed SafeRTOS critical-section binding artifact, completed callback reentrancy/deferred-callback policy artifact, target integration/fault-injection logs | fill `EVID-TGT-003` and `EVID-TGT-007` with target artifacts before claiming multi-task/ISR concurrency safety |

## Waiting / Backlog Items

| Item | Status | Blocker |
| --- | --- | --- |
| Vendor finding export evidence | Waiting | vendor-qualified tool/license/export access; readiness snapshot `EVID-CI-120` confirms baseline is available and vendor export is missing |
| Target-qualified stack/memory-map evidence | Waiting | target-release build/toolchain and qualified analyzer output |
| AM263Px/SafeRTOS porting layer | Planned | target artifact execution pending; planning artifacts are `EVID-TGT-001`, `EVID-TGT-003`, `EVID-TGT-005`, `EVID-TGT-007`, and `EVID-TGT-009` |
| AM263Px/SafeRTOS target evidence | Waiting | target hardware/build environment, completed `EVID-TGT-003`, completed `EVID-TGT-005`, completed `EVID-TGT-007`, SafeRTOS safety evidence, linker/stack/timing capture process, software-vs-hardware CRC equivalence and HW diagnostic capture process |
| Public API critical-section policy | Portable Host Closed | portable contract/startup gate `RV-433`, public API guard application `RV-434`, telemetry snapshot API `RV-435`, outbound queue snapshot/report-refresh/helper/feedback-clear evidence `RV-436`/`RV-439`/`RV-442`, channel-manager snapshot/report-refresh evidence `RV-437`, active-channel snapshot helper evidence `RV-438`, inbound event resolve/record boundary evidence `RV-440`/`RV-441`, channel query/frame receive boundary evidence `RV-443`/`RV-444`, balanced matrix extension `RV-447`, and portable host closeout review `RV-445` are in place; SafeRTOS binding and callback reentrancy policy evidence remain target scope |
| Session reset timer quiescence | Closed | `TC-API-019`/`RV-432`; reset emits supervision/retransmission `CANCEL` commands with `NONE` reason |
| RaSTA PDU/checksum/timestamp parity | Representative Host Closed | selected no-checksum SR host parity is closed by `RV-506..RV-518`; exact official/customer clause mapping, non-none checksum algorithm implementation, and CRC-bearing redundancy behavior remain only if selected |
| RaSTA SCI/application-message aggregation | Scope Decision Needed | current SR data field is treated as bounded payload bytes; implement aggregation only if official/customer scope requires it |
| MAC/security extension | Not Started | controlled requirement needed; otherwise keep `uRequireMac` as explicit unsupported extension gate |
| Future redundancy routing modes | Not Started | policy decision beyond current active-standby baseline |

## Readiness Assessment

| Area | Readiness | Comment |
| --- | --- | --- |
| 설계 구조 | High | 모듈 경계와 책임 분리는 안정적 |
| 단위 테스트 기반 | Medium-High | 핵심 protocol/supervisor/codec 규칙은 representative matrix 수준 |
| 프로토콜 완성도 | Medium-High | 잔여는 broad incompleteness가 아니라 next policy variants |
| 통합 가능성 | Medium-High | 주요 fault-integration family는 연결됨 |
| 인증 증빙 준비 | Medium-High | helper/tooling과 portable API concurrency/reset-quiescence evidence는 보강됐으나 target binding evidence가 필요함 |

## Recommended Next Order

1. `External: Actual Vendor Export Acquisition`
2. `External: First AM263Px/SafeRTOS Target Package Execution`
3. `Local Prep: Fill EVID-TGT-003/005/007 When Target Context Is Known`
4. `Selected Requirement: Non-None Checksum Algorithm Implementation`
5. `Selected Requirement: CRC-Bearing Redundancy PDU Behavior`
6. `Controlled Scope Decision: RaSTA SCI/Application Aggregation`
7. `Selected Requirement: MAC/Security Extension`
8. `Selected Requirement: Protocol Sequencing Variants For New Message/Ordering Family`
9. `Selected Requirement: Redundancy Next Policy Growth`

## Next Gate Definition

- Gate Name: `G-P3-P4-Closeout`
- Gate intent: current representative closeout state를 유지하고, residual을 실제 next policy/security/evidence growth로만 관리한다.
- 통과 조건:
  - `R-001..R-008`이 current closeout state와 실제 residual만 가리킬 것
  - `R-009` portable host API concurrency/reset quiescence는 `RV-445` 기준으로 closeout되어 있고, 남은 concurrency claim은 target-porting evidence scope로 유지될 것
  - accepted V&V follow-up이 baseline defect wording이 아니라 explicit backlog/evidence item으로 유지될 것
  - AM263Px/SafeRTOS work가 core implementation dependency가 아니라 target porting/evidence workstream으로 유지되고, hardware acceleration은 target adapter 뒤에만 위치할 것
  - numeric parity growth는 current target range에서 closeout으로 유지하고, 새 policy/failure-mode가 있을 때만 확장할 것
  - unselected host-only growth는 `RV-530` 기준으로 중단하고, 다음 구현은 external/target artifact 또는 controlled requirement/status/policy selection으로만 열 것
  - build, unit/integration test, `cppcheck`가 깨끗할 것
