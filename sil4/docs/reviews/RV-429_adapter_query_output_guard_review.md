# Review Note - Adapter Query Output Guard

## Document Control

- Review ID: `RV-429`
- Related Test: `TC-PA-013`
- Status: `Closed`
- Date: `2026-06-02`

## Scope

- `sil4/src/rsrx_platform_adapters.c`
- `sil4/tests/unit/test_rsrx_platform_adapters.c`
- `sil4/docs/verification/platform_adapter_layer_test_spec_draft.md`
- `sil4/docs/design/lld/platform_adapter_layer_lld_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Decision

`rsrx_transport_adapter_query_channel` now clears a valid output channel-state pointer at entry to
the `INVALID/0` baseline.

## Rationale

Channel query failure paths must not leave caller-owned availability state referencing a previous
successful query. Clearing the output before validation and before channel-manager refresh avoids
stale channel availability being consumed as the result of a rejected or failed query.

Null output pointers remain invalid arguments and cannot be written.

## Verification

- `TC-PA-013` checks null-context query rejection clears a stale output state.
- `TC-PA-009` now also checks topology-mismatch refresh failure clears the output state.

## Residual

No new residual is opened. Adapter residual remains target runtime binding and selected codec policy
maintenance.
