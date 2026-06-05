# Review Record - RV-443 Channel Query API Boundary

## Scope

- `rsrx_session_query_channel_state`
- channel-query paths in `rsrx_transport_supervisor_poll_receive`
- alternative/refresh channel availability helpers in `rsrx_transport_supervisor.c`
- `TC-API-028`
- `TC-SUP-085`
- `R-009 API concurrency and reset quiescence`

## Findings

- The session API now exposes a bounded transport channel query function inside the public API critical-section boundary.
- Supervisor poll, alternative-channel, and refresh-channel query paths call `rsrx_session_query_channel_state` instead of directly reading session transport-adapter query internals.
- API guard paths reject null/uninitialized session, null output, and critical-section enter/exit failures with `RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT`.
- Invalid and lock-failure paths clear the caller-owned channel state to invalid/unavailable baseline before returning.

## Residual

- This review does not claim SafeRTOS multi-task/ISR safety by itself; target critical-section binding, callback reentrancy/deferred-callback policy, and target integration/fault-injection logs remain required.
- Frame receive remains an explicit supervisor runtime operation and requires target usage-policy evidence if invoked from separate tasks.

## Conclusion

Accepted as a portable host reduction of `R-009` residual for supervisor channel-state query paths.
