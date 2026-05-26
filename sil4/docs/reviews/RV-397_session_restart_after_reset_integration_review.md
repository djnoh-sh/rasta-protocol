# RV-397 Session Restart After Reset Integration Review

## Scope

- Requirement focus: `FR-001`, `FR-003`, `IF-001`
- Verification item: `TC-INT-209`
- Implementation focus: public API reset with transport supervisor integration
- Evidence focus: reset must preserve session/supervisor binding while clearing runtime state enough for a fresh startup/connect/handshake cycle.

## Review Questions

1. Does the integrated session-supervisor path restart after fail-safe timeout and reset using the same context?
2. Are public API callback and diagnostic bindings still active after reset?
3. Can the existing supervisor context pump a post-reset handshake frame and drive the session back to `ESTABLISHED`?

## Findings

- `TC-INT-209` starts, connects, establishes, injects supervision timeout, and resets the same session context.
- The test then restarts and reconnects the session, verifying callback/diagnostic increments and retained transport dispatch.
- The existing supervisor context pumps a post-reset `CONNECT_RESPONSE` frame and returns the session to `RSRX_STATE_ESTABLISHED`.
- No production change is required for this review unit; the existing reset behavior supports the integration restart path.

## Conclusion

Pass. Session reset preserves enough integration binding for the same session-supervisor pair to complete a fresh startup/connect/handshake cycle.

## Residual Risk

- Target ports must preserve the same reset distinction: runtime state is cleared, but configured platform/transport binding remains valid until an explicit reinitialization policy says otherwise.
