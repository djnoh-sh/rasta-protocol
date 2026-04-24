- Scope: `R-003 supervisor holdoff-active telemetry`

## Review Question

1. Does the supervisor report expose whether preferred-recovery holdoff is currently active without requiring callers to infer it from progress/remaining values?
2. Is the mirrored flag aligned with failover, hold, and recovery transitions already covered by switch-audit tests?
3. Do roadmap/spec/traceability updates keep this work inside current redundancy observability hardening rather than reopening numeric holdoff growth?

## Evidence Reviewed

- `sil4/include/rsrx_transport_supervisor.h`
- `sil4/src/rsrx_transport_supervisor.c`
- `sil4/tests/unit/test_rsrx_transport_supervisor.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/design/lld/transport_supervisor_lld_draft.md`
- `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. `rsrx_transport_supervisor_report_t` now exposes `uPreferredRecoveryHoldoffActive`, so callers can distinguish an in-progress holdoff window from mere target/progress counters.
2. The flag is refreshed from the same active/preferred/availability state used for existing switch-audit telemetry, keeping failover inactive, hold active, and recovery inactive transitions consistent.
3. Unit and integration coverage now verify representative `0 -> 1 -> 0` holdoff-active transitions across failover, first hold, and recovery.
4. Documentation and roadmap updates keep the residual wording focused on future redundancy policy growth, not additional threshold-number expansion.

## Conclusion

- Reviewed change is acceptable.
- Residual `R-003` scope remains outside current holdoff-visibility telemetry and stays focused on future policy growth or broader long-run semantics.
