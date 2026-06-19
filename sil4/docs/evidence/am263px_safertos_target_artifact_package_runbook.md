# AM263Px SafeRTOS Target Artifact Package Runbook

## Document Control

- Document ID: `EVID-TGT-009`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This runbook defines the execution order for filling the AM263Px + SafeRTOS target artifact package template `EVID-TGT-005`.

It is intended for the first target build or target-qualified simulator run. It does not claim target readiness until the artifacts are captured, reviewed, and linked back to the roadmap.

## Preconditions

Before starting this runbook, confirm:

1. target board or target-qualified simulator is identified
2. compiler, linker, TI SDK, and SafeRTOS versions are known
3. linker command file and build flags are controlled
4. SafeRTOS task/timer/queue model is defined
5. target transport driver path is selected
6. selected hardware acceleration policy is known, including `Not Selected` when no acceleration is used
7. `EVID-TGT-003`, `EVID-TGT-005`, and `EVID-TGT-007` are available as fill-in templates

## Execution Order

### 1. Capture Build Metadata

Fill `TGT-PKG-001` in `EVID-TGT-005`.

Required capture:

1. commit ID and branch/PR
2. compiler and linker version
3. TI SDK version
4. SafeRTOS version
5. linker command file
6. build profile and flags
7. generated binary identifier

### 2. Capture SafeRTOS Ownership Model

Fill `TGT-PKG-002`.

Required capture:

1. protocol task priority and stack allocation
2. timer ownership and tick/jitter assumptions
3. queue ownership, depth, overflow policy, and static allocation policy
4. startup/shutdown ordering
5. dynamic allocation prohibition or documented deviation

### 3. Fill Critical-Section Binding Evidence

Complete `EVID-TGT-003`, then reference it from `TGT-PKG-003`.

Minimum capture:

1. selected SafeRTOS primitive
2. wrapper source or reviewed snippet
3. task/timer/ISR/deferred-ISR context classification
4. balanced enter/exit instrumentation
5. injected enter/exit failure result

### 4. Fill Callback Reentrancy Policy

Complete `EVID-TGT-007`, then reference it from `TGT-PKG-004`.

Minimum capture:

1. callback ownership matrix
2. prohibited direct callback-to-core paths
3. deferred queue/task notification boundary where required
4. callback reentry attempt handling
5. diagnostics callback mutation policy

### 5. Capture Transport Adapter Binding

Fill `TGT-PKG-005`.

Required capture:

1. selected TI driver or transport stack
2. status-to-`rsrx_transport_status_t` mapping
3. timeout ownership
4. retry ownership
5. transport-down and recovery behavior
6. malformed frame delivery or rejection policy

### 6. Capture Linker Map And Stack Evidence

Fill `TGT-PKG-006` and `TGT-PKG-007`.

Required capture:

1. linker map file from the same target build
2. text/rodata/data/bss/stack/heap region summary
3. per-task stack high-water mark or static analysis result
4. stack margin and measurement workload
5. unresolved call or recursion handling if static analysis is used

### 7. Capture Timing Evidence

Fill `TGT-PKG-008`.

Required capture:

1. supervision pump timing
2. encode/decode timing
3. dispatch timing
4. timer callback timing
5. transport adapter timing
6. measurement method, clock source, and worst-case workload description

### 8. Capture Target Integration Logs

Fill `TGT-PKG-009`.

Minimum scenarios:

1. startup and configuration validation
2. connect request/response and steady-state heartbeat
3. application data send/receive
4. retransmission request path
5. malformed PDU rejection
6. transport down and recovery
7. outbound queue full or backpressure path
8. redundancy failover path if redundant mode is selected
9. reset with supervision/retransmission timer cancel behavior
10. diagnostic capture for each injected fault

### 9. Capture Hardware Acceleration Evidence If Selected

Fill `TGT-PKG-010`.

If hardware acceleration is not selected, record `Not Selected` with the controlled-requirement rationale.

If selected, capture:

1. selected algorithm and peripheral
2. software reference vector log
3. hardware vector log
4. boundary vector log
5. malformed-input vector log
6. timeout or diagnostic fault log
7. fallback or fail-safe policy

### 10. Capture Safety Manual Response

Fill `TGT-PKG-011`.

Required capture:

1. SafeRTOS assumptions and project responses
2. TI SDK/driver assumptions and project responses
3. unresolved constraints
4. deviations or restrictions copied to roadmap residuals

## Closeout Update

After all package rows are filled:

1. create a target package review record
2. update `first_actual_vendor_evidence_set_execution_tracker.md` row `EVS-012`
3. update `roadmap_status.md` `R-008` and `R-009`
4. update `evidence_index.md` with the actual target package artifact
5. do not mark target readiness closed if any required row is `TBD` without a controlled rationale

## Stop Conditions

Stop and leave the package open if:

1. target build metadata is missing
2. stack/map/timing artifacts are not tied to the same build
3. `EVID-TGT-003` or `EVID-TGT-007` is incomplete while concurrency safety is being claimed
4. hardware acceleration is selected but software-vs-hardware vector evidence is missing
5. target logs are replaced by host-only logs
