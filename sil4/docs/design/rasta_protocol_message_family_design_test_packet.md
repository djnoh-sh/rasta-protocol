# RaSTA Protocol Message Family / Sequencing Variant Design/Test Packet

## Document Control

- Document ID: `PROTO-MSG-PARITY-001`
- Version: `0.1.0`
- Status: `Draft / Implementation Blocked`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This packet defines the implementation gate for RaSTA message-family and sequencing variants beyond the current selected host baseline.

The current implementation supports the selected message family used by the existing protocol-context and supervisor paths. `RetrResp` and `RetrData` remain explicit rejected mappings until a controlled requirement selects their behavior.

## Current Baseline

- Numeric RaSTA type constants are represented for the known source-basis message values.
- Supported current host mappings cover the selected control/data family used by the implemented no-checksum SR path.
- Unsupported `RetrResp` and `RetrData` mappings are rejected with typed codec status.
- Protocol context representative coverage closes current sequence, confirmation, retransmission request, recovery, repeated-gap, and unsequenced-family behavior.
- Existing unsupported-message rejection is an intentional safety boundary, not a missing null check or accidental decode failure.

## Missing Controlled Inputs

| Input | Required Detail | Implementation Impact |
| --- | --- | --- |
| Selected family | exact message family to add, such as `RetrResp`, `RetrData`, or another controlled variant | prevents broad message growth |
| Clause mapping | official/customer requirement IDs and state-machine semantics | defines when the message is legal |
| Wire payload layout | fixed and variable fields for the selected message family | defines codec encode/decode contracts |
| Sequencing rule | sequenced, unsequenced, recovery-only, retransmission-pending-only, or mixed behavior | defines protocol-context admission |
| Confirmation rule | allowed confirmed-sequence progression and rejection cases | prevents state regression |
| Runtime handoff | supervisor event mapping, fail-safe trigger, and application callback behavior | defines integration behavior |
| Recovery interaction | relationship to retransmission buffer, repeated-gap state, and cleanup | prevents stale recovery state |
| Negative taxonomy | unsupported state, duplicate, stale, gap, invalid confirmation, malformed payload, and illegal family/state pair | defines diagnostics |
| Vectors | positive and negative encoded SR frames for selected families | required before codec implementation |

## Proposed Design Boundary

The codec owns:

- numeric mapping for a selected RaSTA message type.
- encode/decode payload layout for the selected family.
- stale output clearing for malformed or unsupported variants.
- preserving explicit rejection for unselected families.

The protocol context owns:

- sequence and confirmation admission for the selected family.
- record/update side effects after admission.
- retransmission-pending and recovery-state interaction.
- no acceptance of a family merely because the codec can decode it.

The transport supervisor owns:

- mapping admitted protocol events to session behavior.
- fail-safe escalation when the selected family is illegal in the current state.
- preserving codec and protocol-context diagnostics.

The application/API layer owns:

- exposing only the selected user-visible behavior.
- no direct dependency on wire-family internals.

## Planned Test Matrix

| Test ID | Scope | Expected Coverage |
| --- | --- | --- |
| `TC-PROTO-MSG-001` | codec mapping | unselected `RetrResp`/`RetrData` remain rejected |
| `TC-PROTO-MSG-002` | codec positive | selected family maps to the exact numeric RaSTA type |
| `TC-PROTO-MSG-003` | codec payload | selected family encode/decode preserves payload layout |
| `TC-PC-MSG-001` | protocol context | selected family follows exact sequence admission rule |
| `TC-PC-MSG-002` | protocol context | invalid confirmation or stale sequence is rejected without state mutation |
| `TC-PC-MSG-003` | protocol context | selected family interacts correctly with retransmission-pending state |
| `TC-SUP-MSG-001` | supervisor | admitted family maps to the expected session event |
| `TC-SUP-MSG-002` | supervisor | illegal family/state combination triggers selected diagnostic or fail-safe behavior |
| `TC-INT-MSG-001` | integration | selected family works through codec, protocol context, supervisor, and session state |
| `TC-INT-MSG-002` | integration | malformed selected-family frame is rejected without application delivery |

## Implementation Gate

Source implementation remains blocked until all of the following are available:

1. selected message family and official/customer requirement references.
2. wire payload layout and encoded fixtures.
3. sequencing and confirmation rules.
4. recovery/retransmission interaction policy.
5. codec, protocol-context, supervisor, and API ownership boundaries.
6. positive and negative unit/integration vectors.
7. diagnostic and fail-safe taxonomy for illegal family/state combinations.

## Review Position

The next source-code change should not enable `RetrResp`, `RetrData`, or any new protocol family by default. Unsupported mappings are correct until this packet's controlled inputs define exact behavior.
