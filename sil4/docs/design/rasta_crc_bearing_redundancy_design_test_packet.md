# RaSTA CRC-Bearing Redundancy PDU Design/Test Packet

## Document Control

- Document ID: `RED-CRC-PARITY-001`
- Version: `0.1.0`
- Status: `Draft / Implementation Blocked`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This packet defines the implementation gate for RaSTA redundancy CRC options B-E. It narrows the next work item from a broad "CRC-bearing redundancy PDU" residual into controlled inputs, design boundaries, and a verification matrix.

This packet does not authorize source-code implementation. The current portable baseline remains option A no-CRC redundancy PDU behavior with explicit rejection of options B-E.

## Current Baseline

- The redundancy PDU profile exposes an 8-byte outer header carrying one SR packet.
- Option A with zero CRC bytes is accepted and implemented.
- Options B, C, D, and E are explicitly rejected by profile validation until selected.
- Carried no-checksum SR decode bridging and supervisor runtime selection are implemented for the option A boundary.
- The existing CRC32 calculator wrapper is separate from RaSTA redundancy CRC parity and must not be treated as options B-E conformance.

## Missing Controlled Inputs

| Input | Required Detail | Implementation Impact |
| --- | --- | --- |
| Clause mapping | official/customer requirement IDs for options B-E | prevents speculative option behavior |
| Option layout | exact CRC field position, covered bytes, length inclusion/exclusion, and endian policy | defines encode/decode wire compatibility |
| Width mapping | CRC byte width for each selected option | drives buffer sizing and profile validation |
| CRC parameters | polynomial, initial value, final XOR, reflection, and seed handling per option | drives portable calculator and target accelerator equivalence |
| Sequence interaction | relation between redundancy PDU sequence, carried SR sequence, and duplicate detection | avoids mixing checksum and routing semantics |
| Positive vectors | golden encoded frames and expected CRC values for each selected option | required for implementation acceptance |
| Negative vectors | mismatch, truncated CRC, tampered coverage bytes, unsupported option, and invalid length cases | required for stale-output and diagnostic assertions |
| Runtime ownership | codec-only rejection versus supervisor/channel diagnostic propagation | defines report taxonomy |
| Target evidence | AM263Px CRC hardware accelerator applicability, software-vs-hardware equivalence, self-test, timeout, and diagnostic behavior | required before target-qualified acceleration claim |

## Proposed Design Boundary

The portable codec owns:

- CRC profile admission and exact option-width validation.
- redundancy PDU length validation before carried SR handoff.
- CRC field serialization and parsing once a selected option is defined.
- stale-output clearing on every rejected decode path.
- typed status mapping for unsupported option, invalid length, truncated CRC, and CRC mismatch.

The CRC calculation must be behind an injectable adapter boundary:

- host software implementation remains the reference behavior.
- AM263Px hardware acceleration may be used only in a target adapter.
- software and hardware results must be compared with controlled vectors before target evidence can claim equivalence.

The supervisor owns:

- rejecting outer redundancy CRC failures before passing the carried SR packet to SR decode.
- preserving diagnostic distinction between outer redundancy CRC rejection and inner SR decode rejection.
- reporting channel/runtime context without changing the codec-owned wire status.

## Planned Test Matrix

| Test ID | Scope | Expected Coverage |
| --- | --- | --- |
| `TC-RED-CRC-001` | contract | selected option exposes width, coverage, and calculator profile metadata |
| `TC-CODEC-REDCRC-001` | codec admission | options B-E remain rejected until an exact profile is selected |
| `TC-CODEC-REDCRC-002` | encode positive | selected option encodes a golden CRC-bearing redundancy frame |
| `TC-CODEC-REDCRC-003` | decode positive | selected option decodes a golden CRC-bearing redundancy frame |
| `TC-CODEC-REDCRC-004` | decode negative | CRC mismatch rejects and clears stale decoded output |
| `TC-CODEC-REDCRC-005` | decode negative | truncated CRC rejects before carried SR handoff |
| `TC-CODEC-REDCRC-006` | decode negative | tampered covered bytes reject even when outer length is otherwise valid |
| `TC-SUP-REDCRC-001` | supervisor | outer CRC failure is reported and inner SR decode is not invoked |
| `TC-INT-REDCRC-001` | integration | tampered redundancy frame is rejected through session/supervisor flow |
| `TC-TGT-REDCRC-001` | target evidence | AM263Px hardware CRC and host software CRC match selected vectors |

## Implementation Gate

Source implementation remains blocked until all of the following are available:

1. exact option B-E selection and clause/customer requirement references.
2. wire layout, covered byte range, endian policy, and CRC width per selected option.
3. positive and negative vectors for host verification.
4. expected status taxonomy and supervisor diagnostic propagation.
5. AM263Px/SafeRTOS target evidence plan if hardware CRC acceleration is claimed.

## Review Position

The next local implementation step should not be CRC code. The next useful local step is either filling this packet's controlled inputs from official/customer material or drafting the Parallel Delivery / multi-path merge design/test packet.
