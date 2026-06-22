# AM263Px SafeRTOS Critical-Section Binding Evidence Template

## Document Control

- Document ID: `EVID-TGT-003`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This template defines the evidence that must be captured before the project can claim that the portable `rsrx_critical_section_port_t` seam is correctly bound to AM263Px + SafeRTOS execution contexts.

This is a target-evidence template only. It does not claim that target execution has already occurred.

## Scope Boundary

In scope:

1. mapping of `eEnterSessionCriticalSection` and `eExitSessionCriticalSection` to the selected SafeRTOS primitive
2. allowed caller contexts for public API, protocol task, timer callback, transport receive callback, and ISR/deferred ISR entry
3. nesting, timeout, failure, and diagnostic behavior
4. target fault-injection evidence needed to prove balanced enter/exit behavior under failures

Out of scope:

1. changing portable core locking semantics
2. claiming ISR safety for callbacks that are not explicitly listed as ISR-safe by the selected SafeRTOS primitive and TI driver manuals
3. replacing host `RV-433..RV-447` evidence

## Binding Identification

| Field | Captured Value |
| --- | --- |
| Target board / SoC | `TBD` |
| Compiler / linker command file | `TBD` |
| TI SDK version | `TBD` |
| SafeRTOS version | `TBD` |
| Selected primitive | `TBD` |
| Primitive source file / wrapper | `TBD` |
| `rsrx_critical_section_port_t` instance owner | `TBD` |
| Build configuration that enables this binding | `TBD` |

## Required Mapping Evidence

| Evidence ID | Required Capture | Acceptance Rule |
| --- | --- | --- |
| TGT-CS-001 | wrapper source or reviewed snippet for `enter` | calls the selected SafeRTOS primitive without dynamic allocation and returns typed failure on lock failure |
| TGT-CS-002 | wrapper source or reviewed snippet for `exit` | releases the same primitive, records failure if supported, and never masks an unmatched enter/exit defect |
| TGT-CS-003 | context ownership note | each public API/supervisor/timer/transport entry context is classified as task, timer-service task, ISR, or deferred ISR |
| TGT-CS-004 | ISR policy note | direct ISR calls into core APIs are either prohibited or routed through a deferred task/queue boundary |
| TGT-CS-005 | nesting policy note | nested API entry is either prohibited by design or explicitly supported by the selected primitive and tested |
| TGT-CS-006 | timeout/failure policy | lock acquisition failure maps to an explicit `RSRX_STATUS_*` or `RSRX_TRANSPORT_STATUS_*` path and diagnostic record |
| TGT-CS-007 | balanced instrumentation log | representative success and failure paths show balanced enter/exit depth returning to zero |
| TGT-CS-008 | fault-injection log | injected enter failure and exit failure preserve stale-output clearing and fail-safe behavior |
| TGT-CS-009 | safety manual compliance note | SafeRTOS and TI SDK assumptions for the primitive are listed with project responses |

## Context Classification Matrix

| Entry Point Family | Expected Target Context | Direct Core Call Allowed? | Required Boundary |
| --- | --- | --- | --- |
| Application public API | task context | `TBD` | `rsrx_critical_section_port_t` |
| Protocol supervisor pump | protocol task context | `TBD` | `rsrx_critical_section_port_t` |
| SafeRTOS timer expiry | timer-service task or deferred queue | `TBD` | public API timer expiry boundary |
| Transport receive callback | task/deferred ISR context | `TBD` | public API frame receive boundary |
| Hardware ISR | ISR context | normally no | deferred queue or task notification |
| Diagnostics callback | task context unless proven otherwise | `TBD` | no reentrant core mutation |

## Minimum Target Test Scenarios

1. successful public API call enters and exits once
2. public API enter failure returns typed rejection and leaves session state unchanged
3. public API exit failure clears caller-owned output and returns typed failure where applicable
4. timer expiry and transport receive are serialized through the selected target boundary
5. ISR-originated transport event is deferred before entering core APIs
6. nested/reentrant callback attempt follows the selected policy and is either rejected or safely deferred
7. fault injection confirms final critical-section depth is zero after each scenario

## Required Attachments

Attach or reference:

1. target build metadata
2. SafeRTOS primitive documentation reference used for the decision
3. wrapper implementation file or reviewed source extract
4. test or instrumentation log
5. fault-injection log
6. safety manual compliance note
7. reviewer sign-off

## Completion Rule

This evidence can close the `AM263Px/SafeRTOS Critical-Section Binding Evidence` roadmap item only when all `TGT-CS-*` rows are filled with concrete target artifacts and reviewed. Until then, the project may claim only that the portable critical-section seam exists and is host-verified.
