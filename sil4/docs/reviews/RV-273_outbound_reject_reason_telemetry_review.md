# RV-273 Outbound Reject Reason Telemetry Review

- Date: 2026-04-22
- Scope: `R-004 outbound reject reason telemetry`
- Inputs: `rsrx_platform_adapters.h`, `rsrx_platform_adapters.c`, `rsrx_transport_supervisor.h`, `rsrx_transport_supervisor.c`, `test_rsrx_platform_adapters.c`, `test_rsrx_transport_supervisor.c`, `outbound_application_data_lld_draft.md`, `platform_adapter_layer_lld_draft.md`, `transport_supervisor_lld_draft.md`, `outbound_application_data_test_spec_draft.md`, `transport_supervisor_test_spec_draft.md`, `traceability_matrix_initial.md`, `roadmap_status.md`

## Review Questions

1. Does outbound telemetry report the last reject origin separately from the transport status?
2. Is queue overflow distinguishable from other encode/send rejection paths?
3. Does supervisor report refresh expose the adapter reject reason without introducing stale reason after accepted/queued application submits?

## Findings

1. `rsrx_outbound_send_telemetry_t::eLastRejectReason` now records invalid argument, protocol-context, codec, transport-send, and queue-overflow reject origins.
2. `TC-OUT-017` verifies queue overflow sets `QUEUE_OVERFLOW`, internal deferred dispatch preserves the last reject reason, and a subsequent accepted/queued application submit clears it to `NONE`.
3. `rsrx_transport_supervisor_report_t::eLastOutboundRejectReason` mirrors the adapter telemetry during queue report refresh.
4. `TC-SUP-033` now checks that supervisor busy-reject telemetry includes the last outbound reject reason.

## Decision

- Pass. Current bounded queue reject reporting now preserves queue-overflow origin without changing queue depth or retry policy.

## Residual

- Future R-004 work should focus on retry/runtime-feedback semantics, fairness policy, or configured-capacity policy changes, not current queue-overflow reject-origin observability.
