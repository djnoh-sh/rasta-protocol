# RV-352 Codec Non-Frame Event Status Review

- Review ID: `RV-352`
- Date: `2026-05-08`
- Scope: non-frame transport event codec status.

## Findings

- `TC-CODEC-022` now expects `RSRX_CODEC_STATUS_NON_FRAME_EVENT` when codec decode is directly called with a transport event other than `FRAME_RECEIVED`.
- This does not change the supervisor receive path, which already gates non-frame receive events before codec handoff and reports `NO_FRAME`.
- The direct codec negative vector is now distinct from generic malformed-frame decode errors.

## Residual

- Non-frame direct codec misuse is no longer collapsed into `DECODE_ERROR`.
- Invalid-channel direct codec misuse and actual MAC/timestamp/PDU implementation remain future codec-security growth.
