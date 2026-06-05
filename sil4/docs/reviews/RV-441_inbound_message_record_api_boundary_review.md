# Review Record - RV-441 Inbound Message Record API Boundary

## Scope

- `rsrx_session_record_inbound_message`
- inbound message record path in `rsrx_transport_supervisor_process_frame`
- `TC-API-026`
- `TC-SUP-083`
- `R-009 API concurrency and reset quiescence`

## Findings

- The session API now exposes a bounded inbound-message record function that records accepted decoded inbound messages inside the public API critical-section boundary.
- The transport supervisor accepted-inbound path now calls `rsrx_session_record_inbound_message` instead of directly mutating the session transport-adapter inbound-record internals.
- API guard paths reject null/uninitialized session, null message, and critical-section enter/exit failures with `RSRX_STATUS_INVALID_ARGUMENT`.
- Supervisor record API failure is not ignored; it escalates through `INBOUND_RECORD_FAILED` to avoid processing `VALID_DATA` with stale inbound cache.
- Existing application-data delivery behavior remains covered by the API unit path that records a decoded data message and then processes `VALID_DATA`.

## Residual

- This review does not claim SafeRTOS multi-task/ISR safety by itself; target critical-section binding, callback reentrancy/deferred-callback policy, and target integration/fault-injection logs remain required.
- Adapter query/receive and feedback-clear operations remain explicit supervisor runtime operations and require target usage-policy evidence if invoked from separate tasks.

## Conclusion

Accepted as a portable host reduction of `R-009` residual for accepted inbound-message recording.
