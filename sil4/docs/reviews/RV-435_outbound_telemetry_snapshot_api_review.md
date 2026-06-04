# Review RV-435 - Outbound Telemetry Snapshot API

## Scope

- Implementation focus: `rsrx_session_copy_outbound_telemetry`
- Verification focus: `TC-API-022`
- Risk focus: `R-009` target concurrency residual from internal telemetry pointer exposure

## Review Questions

1. Does the public API provide a caller-owned telemetry snapshot path?
2. Is the snapshot copied under the existing critical-section seam?
3. Does the API clear caller output on invalid argument or lock-enter failure?

## Findings

1. `rsrx_session_copy_outbound_telemetry` copies outbound telemetry into caller-owned storage while the session critical section is held.
2. The function clears the caller buffer to a neutral baseline before validation, preventing stale telemetry reuse on failure.
3. `TC-API-022` verifies successful snapshot contents and lock-enter failure output clearing.
4. The legacy pointer-return telemetry API remains for backward-compatible diagnostic reads, but target multi-task policy can now use the snapshot API instead.

## Decision

Pass. The portable host baseline now has a bounded snapshot API that avoids requiring target code to retain an internal telemetry pointer.

## Residual

- Target SafeRTOS evidence must still prove the critical-section binding and telemetry snapshot call context.
- Transport supervisor report refresh still reads some session adapter fields directly; if that path is used from a concurrent target task, it needs a separate locking/snapshot review.
