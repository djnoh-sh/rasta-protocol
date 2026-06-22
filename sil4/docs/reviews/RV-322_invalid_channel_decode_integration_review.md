# RV-322 Invalid Channel Decode Integration Review

## Scope

- Review ID: `RV-322`
- Scope: `invalid channel decode failure integration`
- Date: 2026-05-06

## Findings

1. `TC-INT-206` drives a real-codec, fake-transport integration path with a valid payload carried on `RSRX_TRANSPORT_CHANNEL_INVALID`.
2. The supervisor reports `RSRX_SUPERVISOR_STATUS_DECODE_FAILED` and preserves `RSRX_SUPERVISOR_DECISION_DECODE_FAILED` without forwarding the frame to the session/application layer.
3. The test verifies the established session remains established, the processed-frame count is not advanced by the rejected frame, and the invalid channel is preserved in the supervisor report.

## Disposition

- Pass.
- Codec channel-admission guard now has integration-level evidence at the supervisor/session boundary.
