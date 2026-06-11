# RV-406 RaSTA SR Timestamp Admission Review

## Document Control

- Review ID: `RV-406`
- Scope: `TC-CODEC-043`, `PDU-PARITY-001F`, `R-006`
- Status: `Accepted`
- Date: `2026-05-28`

## Review Summary

The SIL4 codec now exposes a RaSTA SR timestamp admission boundary for decoded SR packets. The boundary validates packet timestamp, confirmed timestamp, current timestamp, accepted past/future windows, and last accepted timestamp before later session/protocol handoff integration.

This keeps timestamp policy deterministic and testable before it is wired into higher-level state/session flow.

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

- Valid timestamp values within the configured past/future window return `RSRX_CODEC_STATUS_OK`.
- Packet timestamp `0` or current timestamp `0` returns `RSRX_CODEC_STATUS_TIMESTAMP_ZERO`.
- Packet or confirmed timestamp beyond the future window returns `RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE`.
- Packet or confirmed timestamp older than the past window returns `RSRX_CODEC_STATUS_TIMESTAMP_STALE`.
- Packet timestamp not greater than the last accepted timestamp returns `RSRX_CODEC_STATUS_TIMESTAMP_REGRESSED`.
- Window arithmetic overflow/underflow and null arguments return `RSRX_CODEC_STATUS_INVALID_ARGUMENT`.

## Residual

- This is a codec-level admission boundary, not session handoff integration.
- A future step must connect monotonic time, local session policy, and SR decoded packet admission before protocol context event resolution.
- Receiver/sender ID authenticity checks remain open.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-406` is accepted only when that chain is green.
