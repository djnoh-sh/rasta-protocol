# RaSTA SR PDU Wire Profile Draft

## Document Control

- Document ID: `PDU-PARITY-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This draft defines the concrete wire-profile and implementation-status baseline for selected no-checksum RaSTA SR PDU parity in the SIL4 reimplementation. It is intentionally limited to the SR packet layout, selected no-checksum SR host behavior, and option A no-CRC redundancy status; checksum algorithms, target timestamp-source binding, redundancy PDU CRC-bearing behavior, and target acceleration remain separate conditional follow-up items.

This profile does not claim SCI/application-message aggregation parity. The current selected host boundary treats the SR data field as a bounded payload byte sequence; any RaSTA SCI/application aggregation behavior must be opened by a separate controlled scope decision before implementation.

This document is repo-source-derived. It uses the existing open-source implementation as the immediate baseline:

- `src/rasta/headers/rastamodule.h`
- `src/rasta/c/rastamodule.c`
- `src/rasta/headers/rasta_new.h`
- `src/rasta/headers/rastahashing.h`

Before claiming official specification conformance, this draft must be updated with controlled RaSTA specification or customer requirement document IDs and exact clause references.

## Current SIL4 Codec Baselines

The default SIL4 codec profile remains a representative skeleton profile. Its header is 16 bytes:

| Offset | Size | Current SIL4 Field |
| --- | ---: | --- |
| 0 | 1 | internal message type |
| 1 | 1 | internal reason |
| 2 | 2 | reserved |
| 4 | 4 | sequence |
| 8 | 4 | confirmation |
| 12 | 2 | payload length |
| 14 | 2 | reserved |

This layout is useful for bounded codec and supervisor evidence, but it is not the selected RaSTA SR wire-layout profile.

The selected RaSTA SR no-checksum host profile is implemented separately. It uses the 28-byte SR header described below, fixed big-endian field encoding, explicit request/decoded-packet contracts, supported numeric message type mapping, receiver/sender IDs, sequence/confirmed sequence, timestamp/confirmed timestamp, bounded payload, and zero-checksum metadata. Checksum-bearing SR profiles remain conditional follow-up work.

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

The selected no-checksum SIL4 RaSTA SR profile encodes and decodes the supported current numeric type boundary using these repo-source values. `RetrResp` and `RetrData` remain explicit unsupported mappings until their behavior is selected by controlled requirement.

## Byte-Order Policy

The repo-source `shortToBytes` and `longToBytes` helpers currently serialize according to host endian. On little-endian hosts this produces little-endian wire bytes; on big-endian hosts it produces big-endian wire bytes.

The SIL4 implementation does not inherit host-dependent wire encoding. The selected SIL4 RaSTA SR profile uses fixed big-endian byte order for 16-bit and 32-bit SR header fields.

| Policy Item | Decision |
| --- | --- |
| Wire byte order | fixed big-endian |
| Host-endian dependence | prohibited in the SIL4 SR profile |
| Public contract | `D_RSRX_CODEC_RASTA_SR_BYTE_ORDER_BIG_ENDIAN` |
| Helper coverage | `rsrx_codec_write_rasta_sr_uint16`, `rsrx_codec_read_rasta_sr_uint16`, `rsrx_codec_write_rasta_sr_uint32`, `rsrx_codec_read_rasta_sr_uint32` |
| Verification | `TC-CODEC-040` |

This closes the internal byte-order policy for the selected SR encode/decode path. It does not by itself prove interoperability with legacy peers that may depend on the repo-source host-endian behavior. If such compatibility is required, deployment-specific golden vectors must be captured and reviewed before enabling that compatibility path.

## Repo-Source Redundancy PDU Metadata Boundary

The existing RaSTA implementation wraps one SR packet in a redundancy-layer PDU with an 8-byte fixed header followed by the carried SR packet and an optional CRC.

| Offset | Size | Repo-Source Field | Source |
| --- | ---: | --- | --- |
| 0 | 2 | redundancy packet length | `rastaRedundancyPacketToBytes`, `shortToBytes(packet.length)` |
| 2 | 2 | reserve | `rastaRedundancyPacketToBytes`, `shortToBytes(packet.reserve)` |
| 4 | 4 | PDU sequence number | `rastaRedundancyPacketToBytes`, `longToBytes(packet.sequence_number)` |
| 8 | variable | carried SR packet | `rastaModuleToBytes(packet.data, ...)` |
| 8 + SR length | 0, 2, or 4 | redundancy CRC | `packet.checksum_type.width / 8` |

The SIL4 implementation now exposes this as `D_RSRX_CODEC_WIRE_PROFILE_RASTA_REDUNDANCY`, with an 8-byte header, max carried SR frame capacity, and a max 4-byte CRC envelope. CRC option admission currently accepts option A / 0-byte no-CRC and rejects B/C/D/E as unsupported until selected by controlled requirement. Option A no-CRC redundancy PDU encode/decode, carried no-checksum SR decode bridging, and supervisor runtime selection are implemented; CRC-bearing redundancy PDU behavior and RaSTA CRC calculation parity remain follow-up work only if selected.

## Remaining Conditional SIL4 Delta

The selected no-checksum SR host baseline implements the current SR PDU common-field path. Remaining deltas are conditional on a selected checksum/security/redundancy/target requirement:

| Boundary | Required Follow-up |
| --- | --- |
| 28-byte SR header profile | Implemented for no-checksum SR encode/decode; checksum-bearing profiles remain follow-up |
| RaSTA numeric type values | Implemented for supported current inbound/outbound SR message families |
| Receiver/sender ID fields | Implemented in no-checksum SR encode/decode, codec-level identity admission, and supervisor SR runtime identity policy wiring |
| Timestamp/confirmed timestamp fields | Implemented as encoded/decoded fields with codec-level admission boundary, timestamp-admitted handoff mapping, and explicit supervisor runtime SR selection |
| CRC32 wrapper is not SR safety-code parity | Selected default SR checksum profile is no-checksum; explicit unsupported-profile rejection is implemented for MD4/BLAKE2b/SipHash profiles; algorithm implementation remains follow-up only if a non-none profile is selected |
| Redundancy PDU profile | Implemented by `rsrx_codec_get_rasta_redundancy_wire_profile()`, option A no-CRC encode/decode, carried SR decode bridging, supervisor runtime selection, `TC-CODEC-047`, `TC-CODEC-049`, `TC-CODEC-050`, and `TC-SUP-077`; CRC-bearing behavior remains follow-up if selected |
| Redundancy CRC option admission | Implemented for option A accepted and B/C/D/E unsupported by `rsrx_codec_validate_rasta_redundancy_crc_profile()` and `TC-CODEC-048`; actual CRC calculation remains follow-up if selected |
| SCI/application-message aggregation | Not claimed by this PDU wire profile; current host path carries bounded payload bytes only; aggregation behavior requires a separate controlled scope decision |
| Internal reason byte is not RaSTA DiscReq reason parity | Implemented for the current mapping boundary by `rsrx_codec_map_reason_to_rasta_disconnect_reason()` and `TC-CODEC-039`; add more mappings only if a new controlled DiscReq reason boundary is introduced |

## Selected Baseline Implementation Status

1. `PDU-PARITY-001A`: Implemented by `D_RSRX_CODEC_WIRE_PROFILE_RASTA_SR`, `D_RSRX_CODEC_RASTA_SR_HEADER_BYTES`, `D_RSRX_CODEC_RASTA_SR_TIMESTAMP_BYTES`, and `rsrx_codec_get_rasta_sr_wire_profile()`.
2. `PDU-PARITY-001B`: Implemented by `rsrx_rasta_sr_encode_request_t`, `rsrx_rasta_sr_decoded_packet_t`, and `TC-CODEC-038`.
3. `PDU-PARITY-001C`: Implemented by `rsrx_rasta_sr_message_type_t`, `rsrx_rasta_disconnect_reason_t`, mapping APIs, and `TC-CODEC-039`.
4. `PDU-PARITY-001D`: Implemented by `rsrx_codec_encode_rasta_sr_no_checksum()`, `rsrx_codec_decode_rasta_sr_no_checksum()`, and `TC-CODEC-041`.
5. `PDU-PARITY-001E`: Selected no-checksum default profile is implemented by `rsrx_codec_get_rasta_sr_default_checksum_profile()` and `TC-CODEC-046`; unsupported-profile admission boundary is implemented by `rsrx_rasta_sr_checksum_profile_t`, `rsrx_codec_validate_rasta_sr_checksum_profile()`, and `TC-CODEC-042`; actual hash calculation remains conditional on a selected non-none profile.
6. `PDU-PARITY-001F`: Codec-level admission boundary and timestamp-admitted handoff mapping are implemented by `rsrx_rasta_sr_timestamp_admission_policy_t`, `rsrx_codec_validate_rasta_sr_timestamp_admission()`, `rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission()`, `TC-CODEC-043`, and `TC-CODEC-044`; supervisor runtime selection is implemented by `rsrx_transport_supervisor_enable_rasta_sr_runtime()` and `TC-SUP-075`.
7. `RED-PDU-PARITY-001A`: Implemented by `D_RSRX_CODEC_WIRE_PROFILE_RASTA_REDUNDANCY`, `D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES`, `D_RSRX_CODEC_RASTA_REDUNDANCY_MAX_CRC_BYTES`, `rsrx_codec_get_rasta_redundancy_wire_profile()`, `rsrx_codec_validate_rasta_redundancy_crc_profile()`, `rsrx_codec_encode_rasta_redundancy_no_crc()`, `rsrx_codec_decode_rasta_redundancy_no_crc()`, `rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum()`, `rsrx_transport_supervisor_enable_rasta_redundancy_sr_runtime()`, `TC-CODEC-047`, `TC-CODEC-048`, `TC-CODEC-049`, `TC-CODEC-050`, and `TC-SUP-077`.

## Review Position

This profile definition narrows `R-006` from a broad codec/security residual to a concrete SR PDU parity backlog. The no-checksum SR common-header/payload behavioral path, selected no-checksum default profile, unsupported checksum-profile admission boundary, codec-level timestamp admission boundary, timestamp-admitted handoff mapping, supervisor runtime SR selection, codec-level receiver/sender identity admission, supervisor identity policy wiring, redundancy PDU metadata boundary, redundancy CRC option admission boundary, redundancy option A no-CRC encode/decode behavior, carried no-checksum SR decode bridge, and supervisor redundancy runtime selection are implemented. This does not claim MD4/BLAKE2b/SipHash calculation, CRC-bearing redundancy PDU behavior, redundancy CRC calculation, SCI/application-message aggregation parity, target timestamp-source binding, or MAC/security-extension completion.
