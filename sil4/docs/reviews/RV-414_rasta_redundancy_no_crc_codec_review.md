# RV-414 RaSTA Redundancy No-CRC Codec Review

## Document Control

- Review ID: `RV-414`
- Scope: `TC-CODEC-049`, `RED-PDU-PARITY-001A`, `R-006`
- Status: `Accepted`
- Date: `2026-06-01`

## Review Summary

The SIL4 codec now implements RaSTA redundancy PDU encode/decode behavior for CRC option A, the no-CRC redundancy profile.

The implementation serializes and parses the 8-byte redundancy header (`length`, `reserve`, `sequence_number`) and carries one SR packet opaquely. It rejects non-zero reserve bytes, malformed lengths, short carried SR packets, unsupported CRC profiles, non-frame transport events, and invalid transport channels with typed statuses.

## Evidence Reviewed

- Code:
  - `sil4/include/rsrx_codec.h`
  - `sil4/src/rsrx_codec.c`
- Tests:
  - `sil4/tests/unit/test_rsrx_codec.c`
  - `sil4/tests/unit/test_rsrx_codec_contract.c`
- Specifications:
  - `sil4/docs/verification/protocol_codec_test_spec_draft.md`
  - `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
  - `sil4/docs/design/rasta_normative_feature_inventory.md`
  - `sil4/docs/roadmap_status.md`

## Accepted Behavior

- Option A no-CRC redundancy PDU encode/decode round-trips length, reserve, sequence number, and carried SR packet bytes.
- Encode rejects small buffers, non-zero reserve, short carried SR packets, packet length mismatch, null arguments, and unsupported CRC profiles.
- Decode clears stale output on failures and rejects short headers, non-zero reserve, trailing bytes, truncated payloads, non-frame events, invalid channels, and null arguments.
- CRC-bearing redundancy options B/C/D/E remain unsupported by admission policy and are not encoded or decoded.

## Residual

- CRC-bearing redundancy PDU behavior is not implemented and must not be claimed.
- If options B/C/D/E are selected later, implementation must include CRC algorithms, golden vectors, malformed CRC negative vectors, and software-vs-hardware equivalence evidence when target acceleration is used.
- Channel-manager behavioral redundancy policy remains separate from redundancy PDU wire behavior.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-414` is accepted only when that chain is green.
