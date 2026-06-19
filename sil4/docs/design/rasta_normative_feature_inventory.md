# RaSTA Normative Feature Inventory

## Document Control

- Document ID: `INV-RASTA-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

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
| `Standard Parity Decision` | V&V/customer/official input identifies the feature as required before claiming full RaSTA standard parity, but exact clause/vector implementation inputs are still pending | Treat as in-scope for full parity planning; do not implement speculatively before a controlled design/test packet exists |
| `Target Evidence` | Depends on AM263Px/SafeRTOS/toolchain/hardware execution | Keep out of portable core; capture in target evidence package |

## Feature Inventory

| Feature | Source Basis | Classification | Current SIL4 Baseline | Required Next Action |
| --- | --- | --- | --- | --- |
| SR PDU common fields: length, type, receiver ID, sender ID, sequence number, confirmed sequence number, timestamp, confirmed timestamp, data, checksum | `RastaPacket` in `rastamodule.h` | RaSTA Mandatory | Selected no-checksum SR host path implements the 28-byte SR profile, fixed byte order, encode/decode request and decoded packet contracts, receiver/sender IDs, sequence/confirmed sequence, timestamp/confirmed timestamp, payload, and explicit no-checksum metadata | Attach official/customer clause IDs before claiming formal conformance; implement checksum-bearing profiles only if selected |
| RaSTA message types: `ConnReq`, `ConnResp`, `RetrReq`, `RetrResp`, `DiscReq`, `Heartbeat`, `Data`, `RetrData` | `rasta_conn_type` in `rastamodule.h` | RaSTA Mandatory | Numeric type constants and mapping APIs are implemented for the current supported host boundary; unsupported `RetrResp`/`RetrData` remain explicit rejected mappings until their behavior is selected | Keep rejected mappings until a controlled protocol requirement selects the unsupported message families |
| Disconnect reason values | `rasta_disconnect_reason` in `rasta_new.h` | RaSTA Mandatory | Numeric disconnect reason mapping is implemented by `rsrx_codec_map_reason_to_rasta_disconnect_reason()` and `TC-CODEC-039`, including unsupported/null-output clear behavior | Extend mappings only when new internal reasons or controlled DiscReq reason boundaries are introduced |
| SR safety code/checksum | `RastaPacket.checksum`, `rasta_calculate_hash`, `rastahashing.h` | RaSTA Configurable | Current CRC32 wrapper is not the same as SR hash/checksum parity; selected default SR checksum profile is explicitly no-checksum, MD4/BLAKE2b/SipHash profiles are rejected until selected, and `SR-CHECKSUM-PARITY-001` captures the implementation gate | Keep non-none SR checksum calculation closed until selected profile, coverage, key/IV policy, vectors, status taxonomy, and target evidence inputs are supplied |
| SR checksum algorithms: MD4, BLAKE2b, SipHash-2-4 with 0/8/16 byte checksum options | `rasta_hash_algorithm`, `rasta_checksum_type` in `rastahashing.h` | RaSTA Configurable | `sil4` has a selected no-checksum default profile plus admission rejection for unsupported non-none profiles; current CRC32 path is separate; `SR-CHECKSUM-PARITY-001` now defines the controlled-input gate | Define algorithm abstraction and vectors only when a non-none selected profile is required |
| Redundancy PDU fields: length, reserve, PDU sequence number, carried SR packet | `RastaRedundancyPacket` in `rastamodule.h` | RaSTA Mandatory if redundancy layer is in scope | Redundancy no-CRC encode/decode is implemented for option A with an 8-byte header carrying one SR packet, a carried no-checksum SR decode bridge is available, supervisor runtime can select that bridge, and `RED-CRC-PARITY-001` captures the CRC-bearing implementation gate; behavioral channel manager policy remains separate | Fill CRC-bearing redundancy option layout, vectors, and evidence inputs before implementing options B-E |
| Redundancy CRC options a-e | `crc_init_opt_a` through `crc_init_opt_e` in `rastacrc.h` | RaSTA Configurable | Redundancy profile reserves up to 4 CRC bytes; option A / 0-byte no-CRC is accepted, options B/C/D/E are explicitly rejected until selected; current CRC32 wrapper uses standard CRC32 vector and does not cover RaSTA CRC options b-e; `RED-CRC-PARITY-001` now defines the controlled-input gate | Keep CRC option profiles B-E closed until option-specific layout, polynomial, vectors, and target equivalence evidence are supplied |
| Timestamp and confirmed timestamp sequence validation | `RastaPacket.timestamp`, `confirmed_timestamp`, `sr_cts_in_seq`, `cur_timestamp` in analysis docs | RaSTA Mandatory for full SR parity | Codec-level static timestamp admission, handoff mapping, and explicit supervisor runtime SR selection exist; `TIME-PARITY-001` now captures the dynamic supervision implementation gate | Fill dynamic T_max/drift/retransmission-delay policy, runtime ownership, vectors, and target timing evidence before implementation |
| Sequence, confirmed sequence, retransmission, and ordering windows | `sr_sn_in_seq`, `sr_sn_range_valid`, `sr_cs_valid`, retransmission analysis docs | RaSTA Mandatory | Representative sequence/confirmation/retransmission families are implemented and tested | Compare current policy against official/window parameters and add missing boundary vectors |
| Packet authenticity: sender/receiver ID validation | `sr_message_authentic` in analysis docs | RaSTA Mandatory | Codec-level receiver/sender identity admission and supervisor SR runtime identity policy wiring exist | Add only deployment-specific identity configuration evidence |
| Application data payload/message aggregation | `RastaMessageData`, `MAX_APP_MSG_LEN`, factory constructors in analysis docs | RaSTA Mandatory if SCI/application aggregation is in scope | Current payload path supports bounded payload bytes, not multi-application-message aggregation parity; `SCI-SCOPE-001` keeps aggregation scoped out until official/customer input reopens it | Reopen only if official/customer scope requires SCI/application aggregation, then define layout, vectors, ownership, and target evidence before implementation |
| MAC/HMAC-style authentication beyond RaSTA checksum/hash profile | V&V Section 9 standard-parity assessment; exact official/customer clause mapping still required | Standard Parity Decision | `uRequireMac` is rejected because no MAC-capable codec profile exists; this remains valid unsupported-state behavior, not full parity. `MAC-PARITY-001` now captures the implementation gate. | Fill MAC algorithm/key/lifecycle policy, coverage, wire placement, vectors, and target crypto/equivalence evidence before implementation |
| AM263Px hardware CRC/crypto acceleration | Target deployment requirement | Target Evidence | Portable CRC32 calculator injection seam exists | Add target adapter and software-vs-hardware equivalence evidence only in the target package |

## Immediate Backlog Split

1. `PDU-PARITY-001`: Selected no-checksum SR wire layout/profile is implemented for the host baseline; official/customer clause mapping remains external evidence work.
2. `PDU-PARITY-002`: Numeric message type and disconnect reason mapping are implemented for the current supported boundary; add new message-family behavior only if selected.
3. `PDU-PARITY-003`: Sender/receiver RaSTA ID authenticity validation is implemented at codec admission and supervisor policy wiring boundaries; deployment-specific identity configuration evidence remains target/customer scope.
4. `PDU-PARITY-004`: Timestamp and confirmed timestamp fields plus codec admission are implemented; target monotonic source binding remains target evidence scope.
5. `SR-CHECKSUM-PARITY-001`: Keep selected no-checksum default profile closed; non-none MD4/BLAKE2b/SipHash 8-byte or 16-byte profiles remain blocked until exact algorithm/profile, coverage, key/IV, vector, status, and target evidence inputs are supplied.
6. `CRC-PARITY-001`: Keep option A no-CRC accepted and B-E explicitly rejected; `RED-CRC-PARITY-001` is drafted and remains blocked on option layout, vectors, and evidence boundaries.
7. `RED-PDU-PARITY-001`: Redundancy PDU profile metadata, option A no-CRC encode/decode behavior, carried no-checksum SR decode bridging, and supervisor runtime selection are defined; CRC-bearing behavior is gated by `RED-CRC-PARITY-001`.
8. `MAC-PARITY-001`: MAC sign/verify design/test packet is drafted; implementation remains blocked until controlled algorithm/key/lifecycle, coverage, wire-placement, vector, and target-evidence inputs are available.
9. `TIME-PARITY-001`: Dynamic Clock/Time Supervision design/test packet is drafted; implementation remains blocked until controlled T_max/drift/retransmission-delay, runtime ownership, vector, and target timing evidence inputs are available.
10. `RED-MODE-PARITY-001`: Parallel Delivery and receive-side multi-path merge/filtering design/test packet is drafted; active-standby remains only the selected representative baseline until routing, duplicate detection, feedback aggregation, diagnostics, and target evidence inputs are available.

## Current Interpretation

The current `sil4` codec/security baseline is representative-host closed for the selected no-checksum SR PDU boundary with optional CRC32 wrapper evidence, but it should not be described as full checksum-bearing or target-qualified RaSTA conformance yet.

The roadmap should therefore distinguish:

- `RaSTA PDU/checksum/timestamp parity`: selected no-checksum SR host parity is closed; checksum-bearing and target timestamp-source evidence remain conditional residuals.
- `RaSTA SCI/application-message aggregation`: scoped out by `SCI-SCOPE-001` unless controlled official/customer input reopens it.
- `MAC/security extension`: standard-parity decision after V&V Section 9, with implementation gated on controlled algorithm/key/lifecycle and vector inputs.
- `AM263Px/SafeRTOS hardware acceleration`: target adapter/evidence backlog, not portable core logic.

`PDU-PARITY-001A..001F` are now implemented through profile metadata, public SR packet data contracts, numeric type/disconnect reason mappings, fixed big-endian byte-order policy, no-checksum SR common-header/payload encode/decode behavior, selected no-checksum default profile, explicit unsupported checksum-profile admission rejection, codec-level static timestamp/window admission boundary, timestamp-admitted handoff mapping, supervisor runtime SR selection, codec-level receiver/sender identity admission, and supervisor identity policy wiring. `RED-PDU-PARITY-001A` is implemented through redundancy wire-profile metadata, CRC option admission, option A no-CRC encode/decode behavior, carried no-checksum SR decode bridging, and supervisor runtime selection. The remaining open standard-parity work is filling MAC/time/SR-checksum/CRC-bearing redundancy/Parallel Delivery controlled inputs; SCI/application-message aggregation remains excluded unless `SCI-SCOPE-001` is reopened by controlled official/customer input.
