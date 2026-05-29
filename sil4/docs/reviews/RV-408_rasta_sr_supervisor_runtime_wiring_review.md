# RV-408 RaSTA SR Supervisor Runtime Wiring Review

## Document Control

- Review ID: `RV-408`
- Scope: `TC-SUP-075`, `PDU-PARITY-001F`, `R-006`
- Status: `Accepted`
- Date: `2026-05-29`

## Review Summary

The transport supervisor now has an explicit RaSTA SR runtime selection path. When enabled, inbound frames are decoded with the no-checksum RaSTA SR decoder and admitted through the timestamp policy before being handed to the existing session event path. When disabled, the supervisor continues to use the configured legacy codec port.

The runtime path keeps timestamp admission state in the supervisor context and reports whether SR runtime selection is enabled, the configured current timestamp, and the last accepted timestamp.

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

- `rsrx_transport_supervisor_enable_rasta_sr_runtime()` rejects null, uninitialized, zero-current, underflowing past-window, and overflowing future-window policies.
- Enabled SR runtime bypasses the legacy `pfDecode` callback and uses RaSTA SR no-checksum decode plus timestamp-admitted handoff mapping.
- Admitted SR `DATA` frames continue through the existing session/application path.
- Future timestamp frames return `RSRX_SUPERVISOR_STATUS_DECODE_FAILED`, preserve the typed codec status, do not increment processed-frame count, and do not advance the last accepted timestamp.
- The default supervisor runtime remains unchanged until SR runtime is explicitly enabled.

## Residual

- Receiver/sender ID authenticity admission is still open.
- Checksum algorithm implementation remains intentionally unsupported unless a controlled requirement selects a RaSTA SR checksum profile.
- Runtime timestamp source refresh is still deployment policy; this step wires a bounded static policy into the supervisor path, not a target clock binding.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-408` is accepted only when that chain is green.
