# RV-351 Codec Unsupported Reason Status Review

- Review ID: `RV-351`
- Date: `2026-05-08`
- Scope: unsupported reason-code codec status.

## Findings

- `TC-CODEC-020` now expects `RSRX_CODEC_STATUS_UNSUPPORTED_REASON` for both outbound encode and inbound decode paths.
- Unsupported reason-code handling is now distinct from unsupported message-type handling and generic decode errors.
- The max defined reason boundary remains covered by `TC-CODEC-021`.

## Residual

- Reason-code diagnostics are no longer collapsed into `UNSUPPORTED_MESSAGE` or `DECODE_ERROR`.
- Transport-origin decode rejections and actual MAC/timestamp/PDU implementation remain future codec-security growth.
