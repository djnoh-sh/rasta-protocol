# Review Note - RV-430 Adapter Receive Output Guard

## Scope

- `rsrx_transport_adapter_receive_frame`
- `TC-PA-014`

## Change Summary

- The transport adapter receive wrapper now clears a valid output frame pointer at function entry.
- The cleared baseline is:
  - `eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID`
  - `puPayload = NULL`
  - `xPayloadLength = 0`
  - `eEventType = RSRX_TRANSPORT_EVENT_NONE`
- A unit test covers the invalid-context path with a stale frame populated before the call.

## Safety Rationale

- Receive failure paths must not leave a previous payload pointer, length, channel, or event type visible to the caller.
- Clearing at wrapper entry makes invalid argument rejection deterministic and bounds stale-output propagation before platform callback execution.
- The policy matches the existing adapter query output guard pattern from `RV-429`.

## Verification

- Covered by `TC-PA-014` in `test_rsrx_platform_adapters.c`.
- Full host verification was required because production code and tests changed.
