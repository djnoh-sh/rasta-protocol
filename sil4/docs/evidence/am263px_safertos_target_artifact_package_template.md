# AM263Px SafeRTOS Target Artifact Package Template

## Document Control

- Document ID: `EVID-TGT-005`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This template defines the minimum target artifact package required before the project can claim AM263Px + SafeRTOS target readiness.

This is a package template only. It does not claim that the target build, target execution, timing closure, stack closure, or hardware acceleration equivalence has already occurred.

## Package Identification

| Field | Captured Value |
| --- | --- |
| Package ID | `TBD` |
| Target board / SoC | `AM263Px / TBD board` |
| Commit ID | `TBD` |
| Branch / PR | `TBD` |
| Build date | `TBD` |
| Compiler and version | `TBD` |
| TI SDK version | `TBD` |
| SafeRTOS version | `TBD` |
| Linker command file | `TBD` |
| Build profile / flags | `TBD` |
| Reviewer | `TBD` |

## Required Artifact Set

| Evidence ID | Required Artifact | Acceptance Rule |
| --- | --- | --- |
| TGT-PKG-001 | target build metadata | exact commit, toolchain, TI SDK, SafeRTOS, linker command file, and flags are recorded |
| TGT-PKG-002 | SafeRTOS task/timer/queue policy | task priorities, stack allocations, timer ownership, queue ownership, and overflow policy are documented |
| TGT-PKG-003 | completed critical-section binding artifact | `EVID-TGT-003` is filled with target primitive mapping, context classification, and fault-injection evidence |
| TGT-PKG-004 | transport adapter binding | TI driver status, timeout, retry ownership, and fail-safe mapping to `rsrx_transport_port_t` are documented |
| TGT-PKG-005 | target linker map | text/rodata/data/bss/stack/heap regions are traceable to the same target build |
| TGT-PKG-006 | target stack evidence | per-task stack bound or measured high-water mark includes method, workload, and margin |
| TGT-PKG-007 | target timing evidence | supervision pump, encode/decode, dispatch, timer callback, and transport adapter timing are measured or analyzed |
| TGT-PKG-008 | target integration logs | connect, data, retransmission, fail-safe, redundancy, queue-full, transport-down, malformed-PDU, and reset-quiescence flows run on target or target-qualified simulator |
| TGT-PKG-009 | hardware acceleration evidence | if selected, CRC/crypto hardware vectors match portable software reference and diagnostic failures map to typed status or fail-safe |
| TGT-PKG-010 | safety manual compliance note | SafeRTOS and TI SDK assumptions are listed with project responses and unresolved constraints |

## Non-Substitution Rules

The following artifacts do not close this package by themselves:

1. host `cppcheck` output
2. host stack/memory helper output
3. POSIX example smoke logs
4. portable host critical-section tests
5. planning documents without target build metadata

These artifacts may be referenced as supporting evidence, but target readiness requires the target-specific artifacts listed above.

## Target Integration Minimum Scenario List

The integration log must identify the build and include at least:

1. startup and configuration validation
2. connect request/response and steady-state heartbeat
3. application data send/receive
4. retransmission request path
5. malformed PDU rejection
6. transport down and recovery
7. outbound queue full / backpressure path
8. redundancy failover path if redundant mode is selected
9. reset with supervision/retransmission timer cancel behavior
10. diagnostic capture for each injected fault

## Hardware CRC/Crypto Section

Complete this section only if a controlled requirement selects hardware acceleration.

| Field | Captured Value |
| --- | --- |
| Selected algorithm | `TBD` |
| Hardware peripheral / driver | `TBD` |
| Software reference implementation | `TBD` |
| Golden vector log | `TBD` |
| Boundary vector log | `TBD` |
| Malformed-input vector log | `TBD` |
| Timeout / diagnostic fault log | `TBD` |
| Fallback or fail-safe policy | `TBD` |

If no acceleration is selected, record `Not Selected` and keep the portable software path as the active implementation.

## Review Gate

The package can be accepted only when:

1. every required artifact row is either filled with a concrete target reference or explicitly marked `Not Selected` with a controlled-requirement rationale
2. no host-only artifact is used as a substitute for target evidence
3. `EVID-TGT-003` is completed if any multi-task, timer, callback, or ISR/deferred-ISR concurrency claim is made
4. target logs are tied to the same commit/build metadata as the stack, memory, and timing evidence
5. open target constraints are copied into `roadmap_status.md` instead of being treated as closed
