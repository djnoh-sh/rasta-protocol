# RV-534 Protocol Codec LLD RaSTA SR Baseline Alignment Review

## Document Control

- Review ID: `RV-534`
- Scope: `LLD-008`, `PDU-PARITY-001`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The protocol codec LLD still described `rsrx_codec.c` primarily as the default deterministic skeleton codec, while the current implementation also contains selected RaSTA SR no-checksum and option A redundancy-carried SR paths.

This review aligns the LLD with the implemented codec baseline without changing executable behavior.

## Evidence Reviewed

- `sil4/docs/design/lld/protocol_codec_lld_draft.md`
- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/design/rasta_normative_feature_inventory.md`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/include/rsrx_codec.h`
- `sil4/src/rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_codec.c`

## Accepted Position

- The default 16-byte codec path remains implemented and tested for internal bounded codec/supervisor evidence.
- The selected no-checksum RaSTA SR path is also implemented and tested through 28-byte SR encode/decode, numeric mapping, byte-order, timestamp admission, identity admission, and handoff mapping contracts.
- The option A redundancy-carried SR path is implemented and tested for no-CRC redundancy wrapping and SR decode bridging.
- Non-none SR checksum/hash algorithms, CRC-bearing redundancy options, target timestamp-source binding, official/customer clause mapping, and MAC/security extension remain conditional residuals.

## Decision

Accepted as a document-only LLD consistency update. `LLD-008` should describe both the default skeleton path and the selected RaSTA SR host path so the design baseline matches current implementation evidence.

## Verification Position

This is a document-only design consistency cleanup. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
