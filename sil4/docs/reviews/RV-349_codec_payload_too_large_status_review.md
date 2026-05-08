# RV-349 Codec Payload Too Large Status Review

- Review ID: `RV-349`
- Date: `2026-05-08`
- Scope: oversized payload codec status.

## Findings

- `TC-CODEC-011` now expects `RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE` for inbound frames whose declared payload exceeds `D_RSRX_CODEC_MAX_PAYLOAD_BYTES`.
- `TC-CODEC-012` now expects the same status for outbound encode requests whose payload length exceeds the bounded codec payload capacity.
- Unsupported message/reason values remain separate from payload capacity failures.

## Residual

- Oversized payload diagnostics are no longer collapsed into generic decode or unsupported-message status.
- Actual MAC/timestamp/PDU implementation and further stale/tamper taxonomy remain future codec-security growth.
