# RaSTA Standard Parity Scope Decision

## Document Control

- Document ID: `SCOPE-RASTA-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This document records the controlled scope decision opened by V&V Section 9 in `sil4/vv_reports/comprehensive_vv_audit_report_2026-06-02.md`.

The decision separates:

- items accepted as required for a full RaSTA standard parity claim
- current host behavior that remains valid for the selected representative baseline
- inputs required before implementation can be safely started

## Decision Summary

The project will treat the following V&V Section 9 items as full RaSTA standard parity scope, not as optional cleanup:

| Scope Item | Decision | Current Baseline Meaning | Implementation Gate |
| --- | --- | --- | --- |
| MAC generation and verification | In scope for full standard parity | `uRequireMac` rejection remains valid unsupported-state behavior only | official/customer clause mapping, algorithm/key lifecycle decision, test vectors |
| Dynamic Clock/Time Supervision | In scope for full standard parity | static timestamp-window admission remains a codec admission boundary only | official/customer timing clauses, T_max/drift parameters, timer-source policy |
| Redundancy CRC options B-E | In scope for full standard parity when redundancy layer is claimed beyond option A | option A no-CRC behavior remains the selected representative host baseline | CRC option layout, expected vectors, software-vs-hardware equivalence plan |
| Parallel Delivery | In scope for full redundancy parity | active-standby channel selection remains representative-closeout for its selected policy | routing policy, duplicate detection key, merge/drop diagnostics, target soak plan |

## Non-Claim Boundary

The current implementation must not be described as full RaSTA standard parity. It may only be described as:

- selected no-checksum SR host parity
- option A no-CRC redundancy PDU host parity
- static timestamp-window admission and identity admission
- deterministic unsupported-profile rejection for unimplemented MAC/checksum/CRC variants
- active-standby redundancy policy representative closeout

## Implementation Readiness Gates

Before starting source-code implementation for any selected scope item, the project must provide a design/test packet containing:

1. controlled requirement or official/customer clause reference
2. selected wire/profile field layout
3. positive and negative test vectors
4. expected status taxonomy and diagnostic behavior
5. host software behavior and AM263Px/SafeRTOS target adapter/evidence boundary

## Priority Order After This Decision

1. MAC/security sign/verify design packet: drafted as `MAC-PARITY-001`; implementation remains blocked until controlled inputs are filled.
2. Dynamic Clock/Time Supervision design packet: drafted as `TIME-PARITY-001`; implementation remains blocked until controlled inputs are filled.
3. CRC-bearing redundancy PDU Option B-E design packet: drafted as `RED-CRC-PARITY-001`; implementation remains blocked until controlled inputs are filled.
4. Parallel Delivery / multi-path merge design packet.

## Review Position

This decision opens controlled requirement work, but it does not by itself authorize speculative code. Code changes remain blocked until the relevant design/test packet defines exact behavior, vectors, and evidence boundaries.
