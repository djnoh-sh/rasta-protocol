# RaSTA SR Non-None Checksum Design/Test Packet

## Document Control

- Document ID: `SR-CHECKSUM-PARITY-001`
- Version: `0.1.0`
- Status: `Draft / Implementation Blocked`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This packet defines the implementation gate for non-none RaSTA SR checksum/hash profiles. It covers the configurable SR safety-code algorithms visible in the reference repository: MD4, BLAKE2b, and SipHash-2-4 with 8-byte or 16-byte checksum lengths.

This packet does not authorize source-code implementation. The current portable baseline remains selected no-checksum SR encode/decode with explicit rejection of unsupported non-none checksum profiles.

## Current Baseline

- `rsrx_codec_get_rasta_sr_default_checksum_profile()` returns `NONE / 0 bytes`.
- `rsrx_codec_validate_rasta_sr_checksum_profile()` accepts only `NONE / 0 bytes`.
- MD4, BLAKE2b, and SipHash-2-4 profiles are rejected with `UNSUPPORTED_CHECKSUM_PROFILE`.
- SR encode/decode behavior is implemented only for the selected no-checksum profile.
- The current CRC32 wrapper is not RaSTA SR checksum/hash parity.
- CRC-bearing redundancy PDU behavior is separate and gated by `RED-CRC-PARITY-001`.

## Missing Controlled Inputs

| Input | Required Detail | Implementation Impact |
| --- | --- | --- |
| Profile selection | exact algorithm and checksum length combinations to support | prevents implementing unused algorithm variants |
| Clause mapping | official/customer requirement IDs for selected non-none profiles | prevents speculative conformance claims |
| Hash coverage | exact SR bytes covered by the safety code, including length/checksum field inclusion or exclusion | defines encode/decode compatibility |
| Key/IV policy | MD4 initial values, BLAKE2b keying, SipHash key length, storage, lifecycle, and zeroization requirements | defines security/safety ownership |
| Byte order | endian policy for algorithm input vectors and encoded checksum bytes | prevents host-endian ambiguity |
| Positive vectors | golden SR frames and expected checksum bytes for each selected profile | required before implementation acceptance |
| Negative vectors | mismatch, truncated checksum, wrong length, unsupported profile, and malformed but checksum-valid frames | required for stale-output and diagnostic assertions |
| Status taxonomy | checksum mismatch, unsupported profile, invalid profile, truncated checksum, calculator failure | defines codec and supervisor reporting |
| Target evidence | AM263Px hardware crypto/CRC applicability, software-vs-hardware equivalence, self-test, timeout, and diagnostic behavior | required before target acceleration claim |

## Proposed Design Boundary

The codec owns:

- checksum profile validation and selected-profile metadata.
- encode-time checksum calculation and checksum field append.
- decode-time checksum extraction and verification before SR packet handoff.
- stale decoded-output clearing on all rejected checksum-bearing decode paths.
- typed status propagation for unsupported profile, invalid profile, truncated checksum, and mismatch.

The checksum calculator owns:

- algorithm-specific calculation behind an injectable interface.
- deterministic software reference behavior for host verification.
- no direct transport, supervisor, or application side effects.

The supervisor owns:

- selecting the configured SR checksum profile at runtime.
- reporting checksum rejection without masking lower-level codec status.
- not treating checksum-valid malformed SR packets as valid protocol input.

Target adapters own:

- optional AM263Px hardware acceleration behind the calculator interface.
- equivalence evidence against software vectors.
- hardware self-test, timeout, and diagnostic reporting if acceleration is used.

## Planned Test Matrix

| Test ID | Scope | Expected Coverage |
| --- | --- | --- |
| `TC-SR-CHK-001` | profile admission | unsupported non-none profiles remain rejected until selected |
| `TC-SR-CHK-002` | contract | selected profile exposes algorithm, checksum length, and coverage metadata |
| `TC-CODEC-SRCHK-001` | encode positive | selected profile encodes golden checksum-bearing SR frame |
| `TC-CODEC-SRCHK-002` | decode positive | selected profile decodes golden checksum-bearing SR frame |
| `TC-CODEC-SRCHK-003` | decode negative | checksum mismatch rejects and clears stale decoded output |
| `TC-CODEC-SRCHK-004` | decode negative | truncated checksum rejects before packet handoff |
| `TC-CODEC-SRCHK-005` | decode negative | checksum-valid malformed SR preserves inner typed decode status |
| `TC-SUP-SRCHK-001` | supervisor | checksum rejection is reported and protocol context is not updated |
| `TC-INT-SRCHK-001` | integration | checksum-bearing valid data reaches application once |
| `TC-INT-SRCHK-002` | integration | tampered checksum-bearing frame is rejected through session flow |
| `TC-TGT-SRCHK-001` | target evidence | AM263Px hardware and software calculator outputs match selected vectors |

## Implementation Gate

Source implementation remains blocked until all of the following are available:

1. selected algorithm/length combinations.
2. official/customer clause or requirement references.
3. exact byte coverage and checksum serialization rules.
4. key/IV lifecycle and zeroization policy where applicable.
5. positive and negative golden vectors.
6. codec status taxonomy and supervisor diagnostic propagation.
7. target hardware-acceleration evidence plan if acceleration is claimed.

## Review Position

The next source-code change should not implement MD4, BLAKE2b, or SipHash calculation yet. The correct next action is to keep the selected no-checksum baseline and implement a non-none checksum profile only after this packet's controlled inputs are filled.
