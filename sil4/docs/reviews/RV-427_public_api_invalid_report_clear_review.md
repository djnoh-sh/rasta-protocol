# Review Note - Public API Invalid Report Clear

## Document Control

- Review ID: `RV-427`
- Related Test: `TC-API-003`
- Status: `Closed`
- Date: `2026-06-02`

## Scope

- `sil4/src/rsrx_api.c`
- `sil4/tests/unit/test_rsrx_api.c`
- `sil4/docs/verification/public_api_layer_test_spec_draft.md`
- `sil4/docs/design/lld/public_api_layer_lld_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Decision

Public API paths that return a session report pointer now clear any valid report output pointer at
entry. Successful and handled rejected transition paths still assign the current session report
before returning.

## Rationale

Invalid argument, uninitialized-session, and unsupported timer-source guard paths should not leave
a caller-owned output pointer referencing a previous session report. Clearing the pointer before
validation makes rejected calls deterministic and prevents stale runtime feedback from being
consumed as the result of the rejected call.

Null report-output pointers remain invalid arguments and cannot be written.

## Verification

- `TC-API-003` now seeds stale report pointers and checks invalid `start`, `connect`,
  `disconnect`, `process_event`, and `process_timer_expiry` guard paths.

## Residual

No new runtime residual is opened. Broader `R-002` residual remains richer runtime-fault ordering
variants and queue-growth semantics.
