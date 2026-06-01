# RV-416 RaSTA Redundancy Carried SR Decode Review

## Document Control

- Review ID: `RV-416`
- Scope: `TC-CODEC-050`, `RED-PDU-PARITY-001A`, `R-006`
- Status: `Accepted`
- Date: `2026-06-01`

## Review Summary

The codec now provides an explicit bridge from an option A no-CRC RaSTA redundancy PDU to the carried no-checksum SR packet decoder.

The bridge validates the outer redundancy header first, then decodes the carried SR packet through `rsrx_codec_decode_rasta_sr_no_checksum()`. This keeps redundancy-layer parsing and SR-layer parsing separate while providing a direct behavioral path for the selected no-CRC/no-checksum baseline.

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

- `rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum()` rejects null arguments with stale SR output cleared when an output packet is supplied.
- Outer redundancy PDU failures are preserved as typed statuses before any inner SR decode is attempted.
- A valid option A redundancy PDU carrying a no-checksum SR packet decodes to the same SR fields as the direct SR no-checksum decoder.
- Inner carried SR malformed frames, including unsupported SR type, are rejected with the inner SR decoder typed status and cleared SR output.

## Residual

- CRC-bearing redundancy options B/C/D/E remain unsupported and are not decoded by this bridge.
- Checksum-bearing SR profiles remain unsupported unless a controlled requirement selects them.
- The bridge does not select channel-manager routing policy; it only decodes the wire representation.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-416` is accepted only when that chain is green.
