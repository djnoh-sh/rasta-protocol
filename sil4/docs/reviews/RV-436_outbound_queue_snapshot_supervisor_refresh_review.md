# Review Record - RV-436 Outbound Queue Snapshot Supervisor Refresh

## Scope

- `rsrx_outbound_queue_snapshot_t`
- `rsrx_session_copy_outbound_queue_snapshot`
- `vRefreshOutboundQueueTelemetry`
- `TC-API-023`
- `TC-SUP-078`
- `R-009 API concurrency and reset quiescence`

## Review Result

- The public API now exposes a caller-owned outbound queue snapshot containing outstanding send presence, deferred send presence/count, and outbound telemetry.
- The snapshot API clears caller output before validation and clears it again on critical-section exit failure, preventing stale caller-owned data on rejected paths.
- Snapshot capture is performed inside the existing session critical-section seam, keeping the direct transport-adapter queue reads inside the API-owned guard boundary.
- Transport supervisor outbound queue report refresh now consumes `rsrx_session_copy_outbound_queue_snapshot` instead of directly reading session transport-adapter queue fields or retaining the telemetry pointer API for this report path.
- Existing queue report tests continue to exercise the supervisor success path, and `TC-API-023` covers snapshot success plus enter-failure output clearing.

## Residual

- `RV-436` does not claim SafeRTOS multi-task/ISR safety by itself; target critical-section binding, callback reentrancy/deferred-callback policy, and target integration/fault-injection logs remain required.
- Other supervisor/session interactions that may be used from separate tasks remain subject to the same target concurrency boundary review.
- The legacy outbound telemetry pointer API remains available for backward-compatible diagnostics; target integration policy should prefer caller-owned snapshot APIs.

## Disposition

Accepted as a portable host reduction of `R-009` residual for outbound queue report refresh.
