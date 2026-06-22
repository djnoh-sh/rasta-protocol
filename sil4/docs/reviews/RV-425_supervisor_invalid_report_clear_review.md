# Review Note - Supervisor Invalid Report Clear

## Document Control

- Review ID: `RV-425`
- Related Test: `TC-SUP-072`
- Status: `Closed`
- Date: `2026-06-02`

## Scope

- `sil4/src/rsrx_transport_supervisor.c`
- `sil4/tests/unit/test_rsrx_transport_supervisor.c`
- `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
- `sil4/docs/design/lld/transport_supervisor_lld_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Decision

Supervisor public APIs that return a report pointer now clear any valid report output pointer at
entry. Successful or handled runtime paths still assign the current supervisor report before
returning.

## Rationale

Invalid argument and uninitialized-context guard paths should not leave a caller-owned output
pointer referencing a previous report. Clearing the pointer before validation makes failure
handling deterministic and prevents stale runtime feedback from being consumed as the result of
the rejected call.

Null report-output pointers remain invalid arguments and cannot be written.

## Verification

- `TC-SUP-072` now checks stale report pointer clearing for invalid `process_frame`,
  `poll_receive`, `process_transport_event`, `process_timer_expiry`, and `pump_receive` calls.

## Residual

No new runtime residual is opened. Broader residual remains richer runtime-fault ordering variants
and queue-growth semantics.
