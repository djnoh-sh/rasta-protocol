# RV-346 CRC-Required Integration Policy Review

- Review ID: `RV-346`
- Date: `2026-05-07`
- Scope: required-CRC deployment policy on the CRC32 session-supervisor integration path.

## Findings

- `TC-INT-208` now sets `uRequireCrc=1` while selecting `rsrx_codec_get_crc32_port()`.
- The integration path proves that the CRC-required startup gate does not block the intended CRC32 session/supervisor configuration.
- The same flow still verifies CRC32 handshake success and tampered `DATA` rejection with `RSRX_CODEC_STATUS_CRC_MISMATCH`.

## Residual

- This closes the required-CRC selected-path integration linkage.
- Actual MAC, timestamp, and PDU parity remain future codec-security implementation work.
