# Review Record - RV-444 Frame Receive API Boundary

## Scope

- `rsrx_session_receive_transport_frame`
- frame receive path in `rsrx_transport_supervisor_poll_receive`
- `TC-API-029`
- `TC-SUP-086`
- `R-009 API concurrency and reset quiescence`

## Findings

- The session API now exposes a bounded transport frame receive function inside the public API critical-section boundary.
- Supervisor poll receive calls `rsrx_session_receive_transport_frame` instead of directly invoking the session transport-adapter receive function.
- API guard paths reject null/uninitialized session, null output, and critical-section enter/exit failures with `RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT`.
- Invalid and lock-failure paths clear the caller-owned transport frame to invalid/null/0/none baseline before returning.

## Residual

- This review does not claim SafeRTOS multi-task/ISR safety by itself; target critical-section binding, callback reentrancy/deferred-callback policy, and target integration/fault-injection logs remain required.
- The portable core still requires target usage policy evidence for which task/ISR context owns supervisor polling and transport receive callbacks.

## Conclusion

Accepted as a portable host reduction of `R-009` residual for supervisor frame receive paths.
