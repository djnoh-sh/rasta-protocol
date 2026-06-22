# RaSTA SCI/Application Message Aggregation Scope Decision

## Document Control

- Document ID: `SCI-SCOPE-001`
- Version: `0.1.0`
- Status: `Draft / Scoped Out Until Reopened`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This document records the controlled scope decision for SCI/application-message aggregation in the SIL4 reimplementation.

The current SIL4 claim remains limited to the RaSTA SR data field as a bounded payload byte sequence. It does not claim SCI-P, SCI-LS, or multi-application-message aggregation parity.

## Source Basis

The reference repository contains SCI and application-message aggregation concepts:

- `src/sci/c/*.c` and `src/sci/headers/*.h` implement SCI-P / SCI-LS telegram handling above RaSTA.
- `struct RastaMessageData` represents multiple application messages.
- `createDataMessage()` serializes each application message as a 2-byte length prefix followed by payload bytes.
- `extractMessageData()` parses a data field back into multiple application messages.
- `MAX_APP_MSG_LEN` documents the legacy relationship between SCI PDU length and RaSTA application-message size.

These are real source-basis features, but they are above the current selected SIL4 SR wire-profile claim.

## Decision

SCI/application-message aggregation is not part of the current SIL4 portable core implementation scope.

The current implementation may claim:

- bounded SR payload byte transport.
- selected no-checksum SR common-header encode/decode behavior.
- application data delivery as one bounded payload buffer.

The current implementation must not claim:

- SCI-P or SCI-LS telegram encode/decode parity.
- `RastaMessageData` multi-message aggregation parity.
- per-application-message length-prefix parsing or construction.
- SCI name to RaSTA ID mapping behavior.
- SCI-specific semantic validation, callbacks, or payload factories.

## Reopen Conditions

Implementation may be reopened only if official/customer scope input requires SCI/application aggregation. A reopen packet must provide:

1. requirement or clause references for SCI/application aggregation.
2. selected SCI-P / SCI-LS protocol subset or explicit exclusion.
3. exact application-message length-prefix wire layout and endian policy.
4. maximum message count, per-message length limit, and total SR data limit.
5. malformed aggregation status taxonomy: truncated length, overrun, zero-length message, excessive count, and trailing bytes.
6. ownership boundary between codec, application API, and any SCI layer.
7. positive and negative golden vectors.
8. AM263Px/SafeRTOS target evidence needs if SCI parsing or callbacks are used on target.

## Planned Test Matrix If Reopened

| Test ID | Scope | Expected Coverage |
| --- | --- | --- |
| `TC-SCI-SCOPE-001` | scope guard | current core does not expose SCI/application aggregation APIs or claims |
| `TC-CODEC-SCI-001` | encode positive | selected application-message list encodes with exact length prefixes |
| `TC-CODEC-SCI-002` | decode positive | encoded aggregation payload decodes into the expected message list |
| `TC-CODEC-SCI-003` | decode negative | truncated length prefix is rejected and stale output is cleared |
| `TC-CODEC-SCI-004` | decode negative | declared message length overrun is rejected |
| `TC-CODEC-SCI-005` | decode negative | excessive message count or total payload length is rejected |
| `TC-API-SCI-001` | API | application send/receive boundary preserves message-list ownership and lifetimes |
| `TC-INT-SCI-001` | integration | aggregated application messages are delivered exactly once through SR data |
| `TC-TGT-SCI-001` | target evidence | AM263Px/SafeRTOS callback and buffer ownership policy is captured |

## Review Position

No source implementation should be started for SCI/application aggregation in the current branch. The correct next action is to keep the bounded payload byte baseline and reopen only when official/customer scope input requires SCI/application aggregation parity.
