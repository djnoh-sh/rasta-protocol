# RV-404 RaSTA SR No-Checksum Codec Review

## Document Control

- Review ID: `RV-404`
- Scope: `TC-CODEC-041`, `PDU-PARITY-001D`, `R-006`
- Status: `Accepted`
- Date: `2026-05-28`

## Review Summary

`rsrx_codec_encode_rasta_sr_no_checksum()` and `rsrx_codec_decode_rasta_sr_no_checksum()` add the first behavioral RaSTA SR PDU wire path for the SIL4 codec. The implementation covers the 28-byte common SR header, supported numeric message types, fixed big-endian field order, bounded payload copy, and explicit no-checksum operation.

This is intentionally not a checksum/hash, timestamp-admission, receiver/sender authenticity, redundancy PDU, or MAC/security-extension closeout.

## Evidence Reviewed

- Code:
  - `sil4/include/rsrx_codec.h`
  - `sil4/src/rsrx_codec.c`
- Tests:
  - `sil4/tests/unit/test_rsrx_codec.c`
- Specifications:
  - `sil4/docs/verification/protocol_codec_test_spec_draft.md`
  - `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
  - `sil4/docs/design/rasta_normative_feature_inventory.md`
  - `sil4/docs/roadmap_status.md`

## Accepted Behavior

- Encode accepts only checksum length `0` and no checksum pointer.
- Encoded SR fields use fixed big-endian ordering.
- Declared packet length must match `28 + payload length`.
- Decode rejects malformed frames with typed status for short header, unsupported type, declared-length mismatch, trailing bytes, truncated payload, non-frame transport events, and invalid channel metadata.
- Decode clears stale SR decoded output before returning failure when an output object is supplied.

## Residual

- `PDU-PARITY-001E`: selected checksum/hash profiles or explicit unsupported configured-profile rejection.
- `PDU-PARITY-001F`: timestamp and confirmed-timestamp admission/window validation.
- Receiver/sender ID authenticity checks at protocol/session admission.
- Redundancy PDU wire layout and CRC parity if redundancy wire mode remains in scope.
- Optional MAC/security extension remains out of scope unless introduced by a controlled requirement.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-404` is accepted only when that chain is green.
