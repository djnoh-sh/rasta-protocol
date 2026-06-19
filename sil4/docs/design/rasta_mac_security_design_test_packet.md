# RaSTA MAC Security Design/Test Packet

## Document Control

- Document ID: `MAC-PARITY-001`
- Version: `0.1.0`
- Status: `Draft / Implementation Blocked`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This packet narrows the MAC/security scope accepted by `SCOPE-RASTA-001` into implementation prerequisites. It does not authorize source-code implementation yet.

## Current Baseline

The current implementation intentionally has no MAC-capable profile:

- `rsrx_codec_get_security_capabilities()` reports MAC unavailable.
- `uRequireMac` is rejected during configuration validation.
- selected RaSTA SR host behavior is no-checksum/no-MAC only.

This is valid unsupported-state behavior, but it is not full RaSTA standard parity.

## Required Controlled Inputs

Implementation remains blocked until the project has the following inputs:

| Input | Required Content | Current Status |
| --- | --- | --- |
| Clause mapping | official/customer clause ID that requires MAC, accepted algorithm set, field placement, and truncation rules | Missing |
| Algorithm selection | MD4, BLAKE2b, or another approved profile, including output length and truncation policy | Missing |
| Key lifecycle | key source, key length, initialization timing, rotation policy, zeroization responsibility, and target storage boundary | Missing |
| MAC coverage | exact byte range covered by the MAC, including whether the length/type/header/payload/checksum fields are included or excluded | Missing |
| Wire placement | whether MAC occupies the RaSTA SR safety-code/checksum field or a separate controlled extension profile | Missing |
| Golden vectors | at least one positive vector and representative negative vectors for mismatch, truncation, wrong key, wrong sender/receiver, replay/timestamp interaction | Missing |
| Target evidence | software-vs-hardware equivalence method if AM263Px/SafeRTOS crypto acceleration is used | Missing |

## Proposed Implementation Boundary

The future implementation should be structured behind a codec/security adapter boundary:

| Boundary | Proposed Rule |
| --- | --- |
| Portable core | owns deterministic profile validation, buffer sizing, status mapping, and stale-output clearing |
| Software MAC adapter | provides a portable reference implementation or wrapper with deterministic test vectors |
| Target crypto adapter | may use AM263Px hardware acceleration only if software-vs-hardware equivalence evidence is captured |
| Configuration validator | accepts `uRequireMac` only when a supported MAC profile and key policy are configured |
| Codec decode | verifies MAC before mapping the packet into protocol/session state |
| Codec encode | calculates MAC after all covered fields are serialized and before the frame is handed to transport |

## Future Status Taxonomy

The current `rsrx_codec_status_t` has no MAC-specific result. A future implementation packet should add or map the following statuses before code changes:

| Future Condition | Required Handling |
| --- | --- |
| unsupported MAC profile | deterministic startup/config rejection |
| null MAC adapter or key material | invalid configuration or invalid argument, depending on boundary |
| MAC field truncated | typed decode rejection with stale decoded output cleared |
| MAC mismatch | typed decode rejection with stale decoded output cleared |
| MAC calculation failure | encode/decode failure without partially valid output |

## Planned Test Matrix

| Planned TC ID | Purpose | Expected Evidence |
| --- | --- | --- |
| `TC-CODEC-MAC-001` | supported MAC profile contract exposes algorithm, output length, coverage, and wire placement | header/contract test |
| `TC-CFG-MAC-001` | `uRequireMac` remains rejected when no MAC profile/key policy is configured | config validator test |
| `TC-CFG-MAC-002` | `uRequireMac` is accepted only after a supported profile and key policy exist | config validator test |
| `TC-CODEC-MAC-002` | encode appends/calculates MAC for the selected profile | positive golden vector |
| `TC-CODEC-MAC-003` | decode accepts a valid MAC frame | positive golden vector |
| `TC-CODEC-MAC-004` | decode rejects MAC mismatch and clears stale output | negative vector |
| `TC-CODEC-MAC-005` | decode rejects truncated MAC and preserves typed status | negative vector |
| `TC-CODEC-MAC-006` | wrong key or wrong identity rejects before session handoff | negative vector |
| `TC-SUP-MAC-001` | supervisor reports MAC decode rejection without accepting inbound protocol state | supervisor unit test |
| `TC-INT-MAC-001` | integration flow rejects tampered MAC frame and remains fail-safe | integration test |
| `TC-TGT-MAC-001` | AM263Px/SafeRTOS hardware/software MAC equivalence | target evidence |

## Implementation Gate

Source-code implementation may start only after this packet is updated with:

1. controlled clause references
2. selected algorithm and output length
3. exact MAC coverage and wire placement
4. key lifecycle and target storage rules
5. golden vectors
6. accepted status taxonomy

## Review Position

This packet converts MAC/security from a broad standard-parity gap into a concrete implementation gate. The next actionable local step is either to fill the missing controlled inputs or to prepare the next design/test packet for Dynamic Clock/Time Supervision.
