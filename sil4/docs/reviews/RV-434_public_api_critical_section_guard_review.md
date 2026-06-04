# Review RV-434 - Public API Critical-Section Guard

## Scope

- Implementation focus: public API shared-state entry points in `rsrx_api.c`
- Verification focus: `TC-API-020`, `TC-API-021`
- V&V focus: 2026-06-02 comprehensive audit `Finding E`

## Review Questions

1. Do initialized public API paths enter the portable critical-section seam before reading or mutating shared session state?
2. Are enter/exit calls balanced on representative success and reject paths?
3. Does lock acquisition failure reject without state, callback, or diagnostic side effects?

## Findings

1. `eEnterSessionCriticalSection` and `eExitSessionCriticalSection` centralize use of `rsrx_critical_section_port_t`.
2. Event-processing APIs, timer expiry ingress, outbound application send, telemetry/state getters, and reset now pass through the critical-section seam after initialization.
3. `TC-API-020` verifies balanced non-nested enter/exit behavior across representative public API calls.
4. `TC-API-021` verifies enter failure blocks `rsrx_session_start` before state transition, callback, diagnostic write, or stale report exposure.

## Decision

Pass for portable host guard scope. `Finding E` is materially reduced because public API shared-state access now uses the required synchronization seam.

## Residual

- Target SafeRTOS binding evidence must prove the port maps to the selected task/ISR-safe primitive.
- Callback reentrancy policy remains explicit residual: current portable baseline verifies balanced guard calls, but target integration must document whether callbacks are non-reentrant, deferred outside the critical section, or protected by a qualified recursive/priority-aware policy.
- `rsrx_session_get_outbound_telemetry` still returns an internal telemetry pointer; target concurrency closeout must define caller lifetime/ownership constraints or add a snapshot-style API before using it as multi-task-safe evidence.
