# Review Record - RV-276 Receive Transport Status Telemetry

## Scope

- Preserve the last receive-side adapter status in the transport supervisor report.
- Extend `TC-SUP-064` coverage from receive-error stage-only telemetry to stage/status telemetry.

## Review Notes

- `eLastReceiveTransportStatus` records the terminal query or receive adapter status observed by `poll_receive`.
- Query-stage failures can now be distinguished by both `CHANNEL_QUERY` stage and the underlying transport status.
- Frame-receive failures retain `FRAME_RECEIVE` stage and the underlying receive status through budgeted and escalated paths.
- Non-error terminal paths keep the existing stage clear behavior while reporting the terminal transport status that caused the path.

## Evidence

- `vTestSupervisorPollReceiveErrorBudgeted`
- `vTestSupervisorPollQueryErrorStageTelemetry`
- `vTestSupervisorPollReceiveErrorEscalatesAndResets`
- `vTestSupervisorPollReceiveRetryOrderingMatrix`
- `TC-SUP-064`

## Decision

- Accepted.

## Residual

- Future `R-002` work should focus on richer runtime-fault variants or queue-growth feedback semantics rather than current query/receive status observability.
