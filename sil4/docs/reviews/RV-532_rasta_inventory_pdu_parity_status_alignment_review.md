# RV-532 RaSTA Inventory PDU Parity Status Alignment Review

## Document Control

- Review ID: `RV-532`
- Scope: `INV-RASTA-001`, `PDU-PARITY-001`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The RaSTA normative feature inventory had stale early-table wording for SR PDU layout, numeric type mapping, and disconnect reason mapping. The document's later current interpretation already stated that `PDU-PARITY-001A..001F` are implemented for the selected no-checksum SR host baseline.

This review aligns the feature inventory and backlog split with the implemented state.

## Evidence Reviewed

- `sil4/docs/design/rasta_normative_feature_inventory.md`
- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/include/rsrx_codec.h`
- `sil4/src/rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_transport_supervisor.c`

## Accepted Position

- The selected no-checksum SR host baseline includes the 28-byte SR profile, fixed byte-order helpers, encode/decode request and decoded packet contracts, supported numeric message type mappings, disconnect reason mapping boundary, timestamp admission boundary, identity admission boundary, and supervisor runtime selection.
- Remaining checksum work is limited to non-none SR checksum/hash calculation only when selected by controlled requirement.
- Remaining redundancy CRC work is limited to CRC-bearing redundancy PDU behavior only when options B/C/D/E are selected.
- Remaining timestamp work is target timestamp-source binding and evidence, not another portable host admission boundary.

## Decision

Accepted as a document-only consistency update. The inventory should describe selected no-checksum SR parity as representative-host closed while preserving explicit residuals for non-none checksum, CRC-bearing redundancy, target timestamp source, official/customer clause mapping, and optional MAC/security extension.

## Verification Position

This is a document-only inventory consistency cleanup. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
