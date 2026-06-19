# RaSTA Dynamic Clock/Time Supervision Design/Test Packet

## Document Control

- Document ID: `TIME-PARITY-001`
- Version: `0.1.0`
- Status: `Draft / Implementation Blocked`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This packet narrows the Dynamic Clock/Time Supervision scope accepted by `SCOPE-RASTA-001` into implementation prerequisites. It explicitly separates dynamic supervision from the current static timestamp admission checks.

## Current Baseline

The current implementation provides a codec-level timestamp admission boundary:

- SR timestamp and confirmed timestamp fields are encoded/decoded.
- `rsrx_codec_validate_rasta_sr_timestamp_admission()` rejects zero, stale, future, regressed, and causally invalid timestamp values.
- supervisor SR runtime reports the current timestamp policy and last accepted timestamp.

This is a static per-packet admission boundary. It is not dynamic peer clock supervision, T_max monitoring, drift accumulation, or retransmission-delay supervision.

## Required Controlled Inputs

Implementation remains blocked until the project has the following inputs:

| Input | Required Content | Current Status |
| --- | --- | --- |
| Clause mapping | official/customer clause ID for dynamic time supervision, including mandatory/optional wording | Missing |
| Time unit and epoch | timestamp unit, wrap behavior, monotonic source relationship, and conversion to platform nanoseconds | Missing |
| T_max policy | maximum tolerated communication delay, budget decomposition, and fail-safe threshold | Missing |
| Drift policy | allowed peer/local clock drift, accumulation window, reset conditions, and hysteresis | Missing |
| Retransmission delay policy | how retransmission timing contributes to dynamic supervision | Missing |
| State ownership | whether supervision state belongs to codec, protocol context, transport supervisor, or session runtime | Missing |
| Diagnostic taxonomy | warning versus fail-safe thresholds, report fields, and event/status mapping | Missing |
| Target evidence | AM263Px/SafeRTOS clock-source, timer-resolution, jitter, WCET, and fault-injection evidence | Missing |

## Proposed Runtime Boundary

Dynamic supervision should not be hidden inside the codec decoder. The future implementation should use an explicit runtime component:

| Boundary | Proposed Rule |
| --- | --- |
| Codec | decodes timestamp fields and performs bounded static admission only |
| Time supervision runtime | maintains peer/local timestamp deltas, drift accumulation, T_max budget, and fail-safe decision state |
| Transport supervisor | calls the time supervision runtime before accepting inbound SR data into protocol/session state |
| Session/API report | exposes last delta, max observed delta, drift counter, supervision state, and last rejection reason |
| Platform adapter | supplies monotonic time and timer evidence without embedding SafeRTOS or AM263Px dependencies in portable core |
| Target package | captures clock source, resolution, jitter, timer callback context, and hardware/software timing evidence |

## Future Status Taxonomy

The current codec statuses cover static timestamp admission only. A future implementation packet should define status/report outcomes for:

| Future Condition | Required Handling |
| --- | --- |
| local clock unavailable | deterministic runtime/config failure before accepting packet |
| peer timestamp delta exceeds T_max | reject packet or fail-safe according to selected policy |
| accumulated drift exceeds threshold | reject packet and escalate according to selected policy |
| retransmission delay exceeds budget | report timing violation and apply selected recovery/fail-safe action |
| timestamp wrap without selected wrap policy | deterministic rejection |
| timing state reset/restart | explicit reset semantics and diagnostic preservation policy |

## Planned Test Matrix

| Planned TC ID | Purpose | Expected Evidence |
| --- | --- | --- |
| `TC-TIME-001` | time supervision profile contract exposes T_max, drift, wrap, and report fields | header/contract test |
| `TC-TIME-002` | nominal peer/local timestamp delta is accepted and tracked | unit test |
| `TC-TIME-003` | delta at T_max boundary is accepted or rejected according to selected policy | boundary test |
| `TC-TIME-004` | delta beyond T_max rejects before session handoff | negative unit test |
| `TC-TIME-005` | accumulated drift above threshold escalates deterministically | long-run unit test |
| `TC-TIME-006` | retransmission delay contributes to the selected timing budget | unit test |
| `TC-TIME-007` | wrap-around is rejected unless a selected wrap policy exists | negative test |
| `TC-SUP-TIME-001` | supervisor reports timing rejection without accepting inbound protocol state | supervisor unit test |
| `TC-INT-TIME-001` | integration flow remains fail-safe after dynamic timing violation | integration test |
| `TC-TGT-TIME-001` | AM263Px/SafeRTOS clock/timer resolution, jitter, and callback evidence are captured | target evidence |

## Implementation Gate

Source-code implementation may start only after this packet is updated with:

1. controlled clause references
2. timestamp unit, epoch, and wrap policy
3. selected T_max and drift parameters
4. retransmission-delay budget policy
5. ownership of runtime state and reset semantics
6. status/report taxonomy
7. target timing evidence plan

## Review Position

This packet converts Dynamic Clock/Time Supervision from a broad V&V parity gap into a concrete implementation gate. The next actionable local step is either to fill the missing controlled inputs or to prepare the CRC-bearing redundancy PDU design/test packet.
