# RaSTA SR PDU Wire Profile Draft

## Document Control

- Document ID: `PDU-PARITY-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-05-26`

## Purpose

This draft defines the first concrete wire-profile target for RaSTA SR PDU parity in the SIL4 reimplementation. It is intentionally limited to the SR packet layout and the staged implementation plan; checksum algorithms, timestamp admission policy, redundancy PDU CRC options, and target acceleration remain separate follow-up items.

This document is repo-source-derived. It uses the existing open-source implementation as the immediate baseline:

- `src/rasta/headers/rastamodule.h`
- `src/rasta/c/rastamodule.c`
- `src/rasta/headers/rasta_new.h`
- `src/rasta/headers/rastahashing.h`

Before claiming official specification conformance, this draft must be updated with controlled RaSTA specification or customer requirement document IDs and exact clause references.

## Current SIL4 Codec Baseline

The current SIL4 codec is a representative skeleton profile. Its header is 16 bytes:

| Offset | Size | Current SIL4 Field |
| --- | ---: | --- |
| 0 | 1 | internal message type |
| 1 | 1 | internal reason |
| 2 | 2 | reserved |
| 4 | 4 | sequence |
| 8 | 4 | confirmation |
| 12 | 2 | payload length |
| 14 | 2 | reserved |

This layout is useful for bounded codec and supervisor evidence, but it is not full RaSTA SR wire-layout parity.

## Repo-Source SR PDU Layout

The existing RaSTA implementation serializes `struct RastaPacket` using a 28-byte fixed header followed by variable data and a configurable safety code/checksum.

| Offset | Size | Repo-Source Field | Source |
| --- | ---: | --- | --- |
| 0 | 2 | packet length | `packFields`, `shortToBytes(packet.length)` |
| 2 | 2 | packet type | `packFields`, `shortToBytes(packet.type)` |
| 4 | 4 | receiver ID | `packFields`, `longToBytes(packet.receiver_id)` |
| 8 | 4 | sender ID | `packFields`, `longToBytes(packet.sender_id)` |
| 12 | 4 | sequence number | `packFields`, `longToBytes(packet.sequence_number)` |
| 16 | 4 | confirmed sequence number | `packFields`, `longToBytes(packet.confirmed_sequence_number)` |
| 20 | 4 | timestamp | `packFields`, `longToBytes(packet.timestamp)` |
| 24 | 4 | confirmed timestamp | `packFields`, `longToBytes(packet.confirmed_timestamp)` |
| 28 | variable | data payload | `rastaModuleToBytes`, `bytesToRastaPacket` |
| 28 + data length | variable | safety code/checksum | `rasta_calculate_hash`, `hashing_context->hash_length * 8` |

The repo-source data length equation is:

```text
data_length = packet.length - 28 - checksum_length
checksum_length = hashing_context->hash_length * 8
```

## Message Type Values

The repo-source `rasta_conn_type` values are:

| Type | Numeric Value |
| --- | ---: |
| `RASTA_TYPE_CONNREQ` | 6200 |
| `RASTA_TYPE_CONNRESP` | 6201 |
| `RASTA_TYPE_RETRREQ` | 6212 |
| `RASTA_TYPE_RETRRESP` | 6213 |
| `RASTA_TYPE_DISCREQ` | 6216 |
| `RASTA_TYPE_HB` | 6220 |
| `RASTA_TYPE_DATA` | 6240 |
| `RASTA_TYPE_RETRDATA` | 6241 |

The SIL4 implementation must either encode these exact numeric values in a RaSTA SR wire profile or provide an explicit conversion layer that proves equivalent wire behavior.

## Endian Policy Gap

The repo-source `shortToBytes` and `longToBytes` helpers currently serialize according to host endian. On little-endian hosts this produces little-endian wire bytes; on big-endian hosts it produces big-endian wire bytes.

The SIL4 implementation should not inherit host-dependent wire encoding. Before implementation, `PDU-PARITY-001` must close one of the following decisions:

| Decision | Effect |
| --- | --- |
| Official spec requires a fixed byte order | Implement that fixed byte order and add cross-host invariant tests |
| Customer deployment accepts repo-source host-dependent behavior | Document this as a compatibility mode and reject mixed-endian deployments |
| Compatibility with existing deployed peers is required | Capture golden vectors from those peers and implement the observed byte order as an explicit profile |

Until this decision is closed, the SR PDU profile can be specified structurally but not claimed wire-compatible.

## Required SIL4 Delta

The current SIL4 codec lacks the following SR PDU parity fields or semantics:

| Gap | Required Follow-up |
| --- | --- |
| No 28-byte SR header profile | Add `RSRX_CODEC_WIRE_PROFILE_RASTA_SR` metadata and tests |
| No RaSTA numeric type values | Add mapping or explicit wire-type conversion |
| No receiver/sender ID fields | Extend encode/decode contracts and authenticity checks |
| No timestamp/confirmed timestamp fields | Extend protocol context validation and replay/window tests |
| CRC32 wrapper is not SR safety-code parity | Add RaSTA checksum/hash profiles or explicit unsupported-profile rejects |
| Internal reason byte is not RaSTA DiscReq reason parity | Add disconnect reason mapping tests |

## Staged Implementation Plan

1. `PDU-PARITY-001A`: Add RaSTA SR profile constants and reporting metadata without changing existing encode/decode behavior.
2. `PDU-PARITY-001B`: Introduce explicit RaSTA SR encode/decode request/result structures containing length, type, IDs, sequence, confirmation, timestamps, payload, and checksum metadata.
3. `PDU-PARITY-001C`: Add RaSTA numeric message-type and disconnect-reason mapping tests.
4. `PDU-PARITY-001D`: Implement no-checksum SR PDU encode/decode once endian policy is closed.
5. `PDU-PARITY-001E`: Add selected SR checksum/hash profiles or startup rejection for unsupported configured profiles.
6. `PDU-PARITY-001F`: Integrate timestamp and confirmed-timestamp validation into protocol context/session admission.

## Review Position

This profile definition narrows `R-006` from a broad codec/security residual to a concrete SR PDU parity backlog. It does not change the current host verification baseline and does not claim checksum, timestamp, redundancy PDU, or MAC/security-extension completion.
