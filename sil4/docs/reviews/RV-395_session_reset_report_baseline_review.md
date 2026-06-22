# RV-395 Session Reset Report Baseline Review

## Scope

- Requirement focus: `FR-003`, `IF-001`
- Verification item: `TC-API-017`
- Implementation focus: `rsrx_session_reset`
- Evidence focus: public API reset must not leave stale fail-safe report data visible to the next startup/reporting cycle.

## Review Questions

1. Does session reset clear a dirty fail-safe `xLastReport` to a neutral baseline?
2. Does reset still return the orchestrator state to `RSRX_STATE_UNINITIALIZED`?
3. Are transition status, reason, diagnostic, dispatched count, and action count all cleared deterministically?

## Findings

- `TC-API-017` drives a session to `ESTABLISHED`, injects a supervision timeout, and confirms the last report contains rejected timeout/error data.
- `rsrx_session_reset` now reuses a single report-baseline helper also used by `rsrx_session_init`.
- After reset, the test verifies `RSRX_STATE_UNINITIALIZED`, `RSRX_STATUS_OK`, `RSRX_REASON_NONE`, `RSRX_DIAG_NONE`, zero dispatched/action counts, and cleared action slots.
- The change preserves existing transport adapter reset and channel manager reset behavior.

## Conclusion

Pass. Public API reset now has implementation and unit evidence that stale fail-safe report data is cleared before the next cycle.

## Residual Risk

- Future additions to `rsrx_orchestrator_report_t` must be added to the shared report-baseline helper.
