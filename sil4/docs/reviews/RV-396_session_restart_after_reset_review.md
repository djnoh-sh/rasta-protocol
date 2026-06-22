# RV-396 Session Restart After Reset Review

## Scope

- Requirement focus: `FR-001`, `FR-003`, `IF-001`
- Verification item: `TC-API-018`
- Implementation focus: `rsrx_session_reset`, `rsrx_session_start`, `rsrx_session_connect`
- Evidence focus: reset must leave the same initialized session context capable of entering a new startup/connect cycle.

## Review Questions

1. Can a session restart after a fail-safe timeout and successful reset without calling `rsrx_session_init` again?
2. Does the post-reset session enter `INITIALIZED` on `rsrx_session_start` and `CONNECTING` on `rsrx_session_connect`?
3. Are callback/executor bindings still usable after reset?

## Findings

- `TC-API-018` drives a session to `ESTABLISHED`, injects a supervision timeout, and resets the same session context.
- The test then calls `rsrx_session_start` and verifies `RSRX_REASON_INIT_COMPLETED`, two dispatched startup actions, and API/diagnostic callbacks.
- The test calls `rsrx_session_connect` next and verifies `RSRX_REASON_CONNECT_REQUESTED`, `RSRX_STATE_CONNECTING`, and retained transport/timer dispatch binding.
- No production change is required for this review unit; the previous reset baseline behavior already supports the restart path.

## Conclusion

Pass. The public API reset path preserves the same initialized session context for a subsequent startup/connect cycle.

## Residual Risk

- Future reset additions must continue to distinguish runtime-state cleanup from configuration/callback binding teardown.
