# RV-410 RaSTA SR Supervisor Identity Policy Wiring Review

## Document Control

- Review ID: `RV-410`
- Scope: `TC-SUP-076`, `PDU-PARITY-003`, `R-006`
- Status: `Accepted`
- Date: `2026-05-29`

## Review Summary

The transport supervisor now supports explicit RaSTA SR receiver/sender identity admission when the SR runtime path is enabled. Matching endpoint IDs continue through the existing session/application path. Mismatched endpoint IDs are rejected before session handoff and reported as typed codec failures.

This completes the current non-cryptographic receiver/sender ID admission path from SR packet decode through supervisor runtime selection.

## Evidence Reviewed

- Code:
  - `sil4/include/rsrx_transport_supervisor.h`
  - `sil4/src/rsrx_transport_supervisor.c`
- Tests:
  - `sil4/tests/unit/test_rsrx_transport_supervisor.c`
- Specifications:
  - `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
  - `sil4/docs/roadmap_status.md`

## Accepted Behavior

- Identity admission cannot be enabled before supervisor init or before SR runtime selection.
- Zero expected receiver/sender IDs are rejected.
- Matching receiver/sender IDs allow admitted SR `DATA` frames to reach session/application handoff.
- Receiver mismatch returns `RSRX_SUPERVISOR_STATUS_DECODE_FAILED` with `RSRX_CODEC_STATUS_RECEIVER_ID_MISMATCH`.
- Rejected identity frames do not advance last accepted timestamp and do not increment processed-frame count.
- Supervisor report telemetry exposes identity admission enabled state and expected endpoint IDs.

## Residual

- This is endpoint ID admission, not cryptographic authentication.
- Selected RaSTA SR checksum/hash implementation remains controlled-requirement dependent.
- MAC/security extension remains out of scope unless explicitly required.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-410` is accepted only when that chain is green.
