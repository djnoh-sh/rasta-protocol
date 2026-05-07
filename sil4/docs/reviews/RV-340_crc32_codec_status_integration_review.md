# RV-340 CRC32 Codec Status Integration Review

- Review ID: `RV-340`
- Date: `2026-05-07`
- Scope: selected CRC32 codec port integration and supervisor codec-status retention.

## Findings

- `TC-INT-208` initializes both session and transport supervisor with `rsrx_codec_get_crc32_port()`.
- The integration flow uses a CRC32-encoded `CONNECT_RESPONSE` to verify that the selected CRC32 port can complete the handshake at the session-supervisor boundary.
- The same flow tampers a CRC32-encoded `DATA` frame and verifies `DECODE_FAILED`, retained `ESTABLISHED` state, no application callback, and `RSRX_CODEC_STATUS_CRC_MISMATCH` in the supervisor report.

## Residual

- This closes the selected CRC32-path integration linkage for representative handshake and tamper rejection.
- Default-vs-CRC deployment policy, MAC/timestamp/PDU parity, and broader stale/tamper status taxonomy remain future codec-security policy growth.
