# RaSTA Parallel Delivery / Multi-Path Merge Design/Test Packet

## Document Control

- Document ID: `RED-MODE-PARITY-001`
- Version: `0.1.0`
- Status: `Draft / Implementation Blocked`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This packet defines the implementation gate for RaSTA Parallel Delivery and receive-side multi-path merge/filtering. It separates the current active-standby representative baseline from a future full redundancy parity claim.

This packet does not authorize source-code implementation. The current portable baseline remains active-standby channel selection with holdoff, failover, recovery, switch audit, and channel-scoped feedback behavior.

## Current Baseline

- The channel manager supports single-channel and active-standby topology.
- `RSRX_TRANSPORT_CHANNEL_REDUNDANT` topology is rejected because no parallel delivery routing mode is implemented.
- Send selection uses one active channel at a time.
- Receive handling is channel-aware but does not merge simultaneous deliveries from multiple channels.
- Existing duplicate/stale sequence rejection belongs to SR protocol ordering and must not be treated as a complete multi-path merge policy.

## Missing Controlled Inputs

| Input | Required Detail | Implementation Impact |
| --- | --- | --- |
| Clause mapping | official/customer requirement IDs for Parallel Delivery and receive-side merge/filtering | prevents speculative routing behavior |
| Routing mode | whether send duplicates every SR/redundancy frame to all available channels or a configured subset | defines transport adapter fan-out semantics |
| Delivery identity | duplicate detection key: redundancy PDU sequence, SR sequence, channel id, timestamp, checksum/MAC outcome, or selected combination | defines receive-side merge correctness |
| Acceptance order | first-valid wins, preferred-channel wins, quorum/consensus, or deterministic priority rule | defines safety behavior under disagreement |
| Mismatch taxonomy | same sequence with different payload, invalid CRC/MAC on one path, stale late duplicate, channel identity mismatch | defines fail-safe versus discard diagnostics |
| Feedback aggregation | how send completion/failure across multiple channels affects outstanding state and retry budget | prevents current single-active budget rules from being reused incorrectly |
| Runtime telemetry | counters and reports for fan-out, duplicate drop, mismatch reject, channel disagreement, and late delivery | defines observability requirements |
| Queue interaction | whether one application message consumes one global outstanding slot or one per-channel outstanding slot | defines bounded queue/backpressure semantics |
| Target evidence | AM263Px/SafeRTOS multi-driver concurrency, callback ordering, ISR/task handoff, timing, and soak requirements | required before target-qualified multi-path claim |

## Proposed Design Boundary

The channel manager owns:

- topology admission for a selected parallel-capable redundancy mode.
- available-channel set calculation.
- deterministic routing-set selection.
- merge-policy state that is independent from active-standby switch audit state.

The transport supervisor owns:

- fan-out scheduling across selected channels.
- aggregation of per-channel send completion/failure into one application-level result.
- receive-side merge/drop decisions before protocol-context recording.
- runtime telemetry for duplicate, mismatch, and late-delivery outcomes.

The protocol context owns:

- SR sequence and confirmation validity after a frame is admitted by the merge policy.
- rejection of protocol-level stale/gap/invalid ordering.
- no channel-specific duplicate merge state.

The platform/transport adapter owns:

- per-channel send/receive execution only.
- no cross-channel duplicate acceptance policy.
- no safety decision based on channel priority or quorum.

## Planned Test Matrix

| Test ID | Scope | Expected Coverage |
| --- | --- | --- |
| `TC-REDMODE-001` | config/channel manager | selected parallel-capable topology is admitted only when policy inputs are defined |
| `TC-REDMODE-002` | channel manager | routing-set selection is deterministic for available primary and secondary channels |
| `TC-SUP-REDMODE-001` | supervisor send | one outbound application frame fans out to the selected channel set |
| `TC-SUP-REDMODE-002` | supervisor send | per-channel send completion/failure aggregates into one bounded application result |
| `TC-SUP-REDMODE-003` | supervisor receive | first valid duplicate is admitted and later identical duplicate is dropped with telemetry |
| `TC-SUP-REDMODE-004` | supervisor receive | same sequence with different payload is rejected or escalated according to selected policy |
| `TC-SUP-REDMODE-005` | supervisor receive | one invalid CRC/MAC path and one valid path follow the selected acceptance rule |
| `TC-SUP-REDMODE-006` | supervisor receive | stale late duplicate does not perturb protocol context state or queue telemetry |
| `TC-INT-REDMODE-001` | integration | parallel send plus duplicated receive preserves one application delivery |
| `TC-INT-REDMODE-002` | integration | channel disagreement enters fail-safe or diagnostic state according to selected policy |
| `TC-TGT-REDMODE-001` | target evidence | AM263Px/SafeRTOS target run proves callback ordering, timing, and soak stability |

## Implementation Gate

Source implementation remains blocked until all of the following are available:

1. exact Parallel Delivery and merge/filtering requirement references.
2. selected routing mode and duplicate detection key.
3. acceptance rule for identical duplicates, mismatched duplicates, and partially invalid paths.
4. send feedback aggregation and bounded queue semantics.
5. runtime telemetry/status taxonomy and fail-safe escalation policy.
6. AM263Px/SafeRTOS target evidence plan for multi-channel callback ordering, timing, and soak behavior.

## Review Position

The next source-code change should not add a parallel-delivery mode yet. A future implementation must first fill this packet's controlled inputs, because active-standby send/receive budget semantics are not safely reusable for simultaneous multi-path delivery.
