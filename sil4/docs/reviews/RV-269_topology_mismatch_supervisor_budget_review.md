# RV-269 Topology Mismatch Supervisor Budget Review

- Date: 2026-04-22
- Scope: `R-002/R-003 topology mismatch query-stage receive budget integration`
- Inputs: `test_rsrx_session_supervisor_flow.c`, `transport_supervisor_lld_draft.md`, `integration_harness_test_spec_draft.md`, `traceability_matrix_initial.md`, `roadmap_status.md`

## Review Questions

1. Does an adapter-origin topology mismatch reach the supervisor as a query-stage receive fault?
2. Does the supervisor apply the existing receive-error budget and escalation semantics?
3. Does the fault avoid receive-path side effects when channel query fails?

## Findings

1. `TC-INT-200` injects mismatched transport-reported channel ids after an active-standby session reaches `ESTABLISHED`.
2. The first mismatched query is budgeted as `RECEIVE_ERROR_BUDGETED` while the session remains `ESTABLISHED`.
3. The second mismatched query escalates to `PROTOCOL_ERROR`, reaches `SAFE_DISCONNECT`, and records `RECEIVE_ERROR_ESCALATED`.
4. The test confirms receive is not called after the query-stage fault and the active channel remains the configured primary before escalation.

## Decision

- Pass. Adapter topology mismatch propagation is covered through the supervisor receive-error budget and fail-safe path.

## Residual

- Future work may add a dedicated topology-fault decision type if diagnostics require a more specific runtime fault origin than receive-error escalation.
