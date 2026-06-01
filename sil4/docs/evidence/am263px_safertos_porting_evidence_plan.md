# AM263Px SafeRTOS Porting Evidence Plan

## Document Control

- Document ID: `EVID-TGT-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-01`

## Purpose

This plan defines how the portable SIL4 protocol core will be ported to an AM263Px + SafeRTOS target without introducing target dependency into the core implementation.

The plan is evidence planning only. It does not claim target execution, target timing closure, stack closure, or hardware CRC/crypto equivalence by itself.

## Scope Boundary

In scope:

1. SafeRTOS task, timer, queue, and critical-section binding expectations
2. TI driver transport adapter expectations
3. optional hardware-backed CRC/crypto adapter expectations
4. target stack, memory, timing, integration, and diagnostic evidence requirements

Out of scope:

1. modifying portable core modules for AM263Px-specific headers or drivers
2. claiming hardware acceleration equivalence without software-vs-hardware vector evidence
3. replacing current host verification with target evidence

## Porting Architecture

| Layer | Target Responsibility | Core Boundary Rule |
| --- | --- | --- |
| SafeRTOS task layer | own protocol task scheduling, supervision cadence, and queue ownership | core APIs remain synchronous and bounded |
| SafeRTOS timer layer | map supervision/retransmission timers to SafeRTOS timer or tick source | timer callbacks enter through existing API/adapter boundary |
| SafeRTOS queue layer | marshal application and transport events without dynamic allocation | queue depth and overflow policy must be configured and tested |
| Critical section layer | protect shared adapter state where required by target concurrency model | no global core lock is introduced into portable modules |
| TI driver transport adapter | bind Ethernet/UART/other selected transport to `rsrx_transport_port_t` semantics | transport status must map to typed protocol/runtime status |
| Target codec/security adapter | optionally use AM263Px CRC/crypto acceleration behind codec/security callback seams | portable software fallback remains available and testable |

## Required Target Evidence

| Evidence ID | Required Artifact | Acceptance Rule |
| --- | --- | --- |
| TGT-001 | target build metadata | commit, compiler, TI SDK, SafeRTOS version, linker command file, build flags |
| TGT-002 | SafeRTOS task model note | protocol task priorities, stack allocation, queue ownership, timer ownership |
| TGT-003 | target linker map | text/rodata/data/bss/stack/heap regions tied to the same build |
| TGT-004 | target stack analysis | per-task stack bound or measured high-water result plus method and margin |
| TGT-005 | timing/WCET evidence | supervision loop, encode/decode, dispatch, and transport adapter timing under target build |
| TGT-006 | target integration log | representative connect/data/retransmission/fail-safe/redundancy flows executed on target or target-qualified simulator |
| TGT-007 | transport driver mapping | TI driver status-to-protocol status mapping, timeout behavior, retry ownership |
| TGT-008 | hardware CRC/crypto equivalence | software-vs-hardware vector logs for every selected accelerated algorithm |
| TGT-009 | hardware diagnostic handling | accelerator self-test, timeout, fault injection, and fallback/degrade behavior |
| TGT-010 | safety manual compliance note | SafeRTOS and TI SDK safety manual assumptions, constraints, and project responses |

## Hardware CRC/Crypto Acceleration Policy

AM263Px hardware acceleration may be used only behind a target-specific codec/security adapter.

Minimum rules:

1. the selected algorithm must already be a controlled requirement
2. the portable software implementation remains the reference path
3. hardware and software outputs must match on golden vectors, boundary vectors, and malformed-input negative vectors
4. hardware timeout or diagnostic failure must map to an explicit typed status or configured fail-safe path
5. target evidence must record whether the target build selected software, hardware, or fallback mode

This policy applies to future CRC-bearing redundancy options and any future non-none SR checksum or MAC profile.

## SafeRTOS Integration Checklist

Before claiming target readiness, record:

1. protocol task priority and rationale
2. timer source, tick rate, and jitter budget
3. queue depth, overflow policy, and backpressure behavior
4. critical sections or mutexes used by adapters
5. stack allocation per task and measured/analysed margin
6. static allocation policy and dynamic allocation prohibition or deviation
7. startup/shutdown sequencing with config validation
8. fault injection results for transport down, malformed PDU, queue full, timeout, and hardware accelerator fault

## Roadmap Impact

This plan moves the AM263Px/SafeRTOS workstream from an undefined not-started item to a defined target-evidence backlog.

It does not reduce the residual until actual target artifacts are attached and reviewed.
