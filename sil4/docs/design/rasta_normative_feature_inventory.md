# RaSTA Normative Feature Inventory

## Document Control

- Document ID: `INV-RASTA-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-01`

## Purpose

This inventory separates RaSTA-defined protocol parity work from project-specific security extensions. It is intended to prevent optional or extension work from being mixed with mandatory RaSTA reimplementation work.

The current source basis is the repository's existing RaSTA implementation and analysis documents:

- `src/rasta/headers/rastamodule.h`
- `src/rasta/headers/rastahashing.h`
- `src/rasta/headers/rastacrc.h`
- `md_doc/codebase_detailed_analysis.md`
- `sil4/docs/design/lld/protocol_codec_lld_draft.md`

If an official RaSTA specification PDF or controlled customer requirement set is later added to the evidence set, this inventory must be updated with exact document IDs, clause IDs, and mandatory/optional wording.

## Classification Rules

| Classification | Meaning | Implementation Policy |
| --- | --- | --- |
| `RaSTA Mandatory` | Defined by RaSTA packet/runtime behavior and required for protocol parity | Implement in portable core or a mandatory selected codec/profile path |
| `RaSTA Configurable` | Defined by RaSTA but selected by configuration/profile | Implement selectable profiles and reject unsupported configured selections |
| `Project Extension` | Not currently evidenced as RaSTA mandatory; project or deployment security addition | Keep behind explicit config/adapter boundary, do not imply RaSTA parity |
| `Target Evidence` | Depends on AM263Px/SafeRTOS/toolchain/hardware execution | Keep out of portable core; capture in target evidence package |

## Feature Inventory

| Feature | Source Basis | Classification | Current SIL4 Baseline | Required Next Action |
| --- | --- | --- | --- | --- |
| SR PDU common fields: length, type, receiver ID, sender ID, sequence number, confirmed sequence number, timestamp, confirmed timestamp, data, checksum | `RastaPacket` in `rastamodule.h` | RaSTA Mandatory | Current skeleton codec has message type, reason, sequence, confirmation, payload, reserved bytes; it does not yet claim full RaSTA SR PDU layout parity | Define `RSRX_CODEC_WIRE_PROFILE_RASTA_SR` with exact field order/size/endian and add encode/decode parity tests |
| RaSTA message types: `ConnReq`, `ConnResp`, `RetrReq`, `RetrResp`, `DiscReq`, `Heartbeat`, `Data`, `RetrData` | `rasta_conn_type` in `rastamodule.h` | RaSTA Mandatory | Current message/event family is representative but not full numeric RaSTA type parity | Map SIL4 `rsrx_message_type_t` to RaSTA numeric type values or introduce explicit RaSTA wire type conversion |
| Disconnect reason values | `rasta_disconnect_reason` in `rasta_new.h` | RaSTA Mandatory | Internal reason taxonomy exists, but numeric RaSTA disconnect reason parity is not separately asserted | Add disconnect-reason parity mapping table and encode/decode tests |
| SR safety code/checksum | `RastaPacket.checksum`, `rasta_calculate_hash`, `rastahashing.h` | RaSTA Configurable | Current CRC32 wrapper is not the same as SR hash/checksum parity; selected default SR checksum profile is explicitly no-checksum, and MD4/BLAKE2b/SipHash profiles are rejected until selected by controlled requirement | Implement non-none SR checksum profile calculation only if a controlled requirement selects one |
| SR checksum algorithms: MD4, BLAKE2b, SipHash-2-4 with 0/8/16 byte checksum options | `rasta_hash_algorithm`, `rasta_checksum_type` in `rastahashing.h` | RaSTA Configurable | `sil4` has a selected no-checksum default profile plus admission rejection for unsupported non-none profiles; current CRC32 path is separate | Add actual algorithm abstraction and vectors only when a non-none selected profile is required |
| Redundancy PDU fields: length, reserve, PDU sequence number, carried SR packet | `RastaRedundancyPacket` in `rastamodule.h` | RaSTA Mandatory if redundancy layer is in scope | Redundancy wire-profile metadata is now defined for an 8-byte header carrying one SR packet plus CRC option envelope; behavioral channel manager policy remains separate | Add redundancy PDU encode/decode/admission tests if redundancy wire mode is in scope |
| Redundancy CRC options a-e | `crc_init_opt_a` through `crc_init_opt_e` in `rastacrc.h` | RaSTA Configurable | Redundancy profile reserves up to 4 CRC bytes; current CRC32 wrapper uses standard CRC32 vector and does not cover RaSTA CRC options b-e | Implement CRC option profiles a-e or reject unsupported configured options with explicit config validation |
| Timestamp and confirmed timestamp sequence validation | `RastaPacket.timestamp`, `confirmed_timestamp`, `sr_cts_in_seq`, `cur_timestamp` in analysis docs | RaSTA Mandatory for full SR parity | Codec-level timestamp admission, handoff mapping, and explicit supervisor runtime SR selection exist | Define target monotonic source refresh policy and extend replay/window tests as target evidence |
| Sequence, confirmed sequence, retransmission, and ordering windows | `sr_sn_in_seq`, `sr_sn_range_valid`, `sr_cs_valid`, retransmission analysis docs | RaSTA Mandatory | Representative sequence/confirmation/retransmission families are implemented and tested | Compare current policy against official/window parameters and add missing boundary vectors |
| Packet authenticity: sender/receiver ID validation | `sr_message_authentic` in analysis docs | RaSTA Mandatory | Codec-level receiver/sender identity admission and supervisor SR runtime identity policy wiring exist | Add only deployment-specific identity configuration evidence |
| Application data payload/message aggregation | `RastaMessageData`, `MAX_APP_MSG_LEN`, factory constructors in analysis docs | RaSTA Mandatory if SCI/application aggregation is in scope | Current payload path supports bounded payload bytes, not multi-application-message aggregation parity | Decide SCI/application aggregation scope; implement if in scope, otherwise document exclusion |
| MAC/HMAC-style authentication beyond RaSTA checksum/hash profile | Not evidenced in current repository as RaSTA mandatory | Project Extension | `uRequireMac` is rejected because no MAC-capable codec profile exists | Keep as extension until a controlled requirement requires it; if required, define algorithm/key/lifecycle policy separately |
| AM263Px hardware CRC/crypto acceleration | Target deployment requirement | Target Evidence | Portable CRC32 calculator injection seam exists | Implement target adapter and prove software-vs-hardware equivalence on target |

## Immediate Backlog Split

1. `PDU-PARITY-001`: Define exact RaSTA SR PDU wire layout profile. Initial repo-source-derived profile draft: `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`.
2. `PDU-PARITY-002`: Add RaSTA numeric message type and disconnect reason mapping.
3. `PDU-PARITY-003`: Add sender/receiver RaSTA ID authenticity validation. Status: codec-level admission boundary and supervisor policy wiring are implemented.
4. `PDU-PARITY-004`: Add timestamp and confirmed timestamp fields plus validation.
5. `CHECKSUM-PARITY-001`: Keep selected no-checksum default profile closed; implement MD4/BLAKE2b/SipHash 8-byte or 16-byte profiles only if a controlled requirement selects one.
6. `CRC-PARITY-001`: Implement or explicitly reject redundancy CRC options a-e.
7. `RED-PDU-PARITY-001`: Redundancy PDU profile metadata is defined; add encode/decode/admission behavior if redundancy wire parity is in current scope.
8. `EXT-MAC-001`: Keep MAC as project extension unless official/customer requirements make it mandatory.

## Current Interpretation

The current `sil4` codec/security baseline is a strong representative skeleton with optional CRC32 evidence, but it should not be described as full RaSTA PDU/checksum/timestamp parity yet.

The roadmap should therefore distinguish:

- `RaSTA PDU/checksum/timestamp parity`: required protocol parity backlog.
- `MAC/security extension`: optional/project-specific unless a controlled requirement says otherwise.
- `AM263Px/SafeRTOS hardware acceleration`: target adapter/evidence backlog, not portable core logic.

`PDU-PARITY-001A..001F` are now implemented through profile metadata, public SR packet data contracts, numeric type/disconnect reason mappings, fixed big-endian byte-order policy, no-checksum SR common-header/payload encode/decode behavior, selected no-checksum default profile, explicit unsupported checksum-profile admission rejection, codec-level timestamp/window admission boundary, timestamp-admitted handoff mapping, supervisor runtime SR selection, codec-level receiver/sender identity admission, and supervisor identity policy wiring. `RED-PDU-PARITY-001A` is started through redundancy wire-profile metadata. The remaining open parity work is non-none checksum/hash calculation only if required, target timestamp source refresh policy, and redundancy PDU encode/decode/admission behavior if that wire mode is in scope.
