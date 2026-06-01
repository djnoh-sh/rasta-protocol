# RV-417 Supervisor Redundancy SR Runtime Wiring Review

## Document Control

- Review ID: `RV-417`
- Scope: `TC-SUP-077`, `RED-PDU-PARITY-001A`, `R-006`
- Status: `Accepted`
- Date: `2026-06-01`

## Review Summary

The transport supervisor can now explicitly select the option A redundancy-carried no-checksum SR runtime path.

`rsrx_transport_supervisor_enable_rasta_redundancy_sr_runtime()` configures the same timestamp admission policy as the direct SR runtime while selecting `rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum()` for inbound frame decode. The existing identity admission gate remains reusable after the redundancy runtime is enabled.

## Evidence Reviewed

- Code:
  - `sil4/include/rsrx_transport_supervisor.h`
  - `sil4/src/rsrx_transport_supervisor.c`
- Tests:
  - `sil4/tests/unit/test_rsrx_transport_supervisor.c`
- Specifications:
  - `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
  - `sil4/docs/design/lld/transport_supervisor_lld_draft.md`
  - `sil4/docs/roadmap_status.md`

## Accepted Behavior

- Direct SR runtime remains the default SR runtime path when `rsrx_transport_supervisor_enable_rasta_sr_runtime()` is used.
- Redundancy-carried SR runtime is selected only by `rsrx_transport_supervisor_enable_rasta_redundancy_sr_runtime()`.
- The redundancy runtime bypasses the legacy codec port and decodes option A no-CRC redundancy PDUs carrying no-checksum SR packets.
- Timestamp and identity admission are still enforced after the carried SR packet is decoded.
- Direct SR frames are rejected in redundancy runtime mode because the selected runtime expects a redundancy PDU envelope.

## Residual

- CRC-bearing redundancy options B/C/D/E remain unsupported.
- Non-none SR checksum profiles remain unsupported unless selected by a controlled requirement.
- Integration-level redundancy-carried SR runtime flow can be added later if this runtime mode becomes the selected deployment profile.

## Verification Position

The required verification chain for code/test changes is `build -> unit/integration tests -> cppcheck`. `RV-417` is accepted only when that chain is green.
