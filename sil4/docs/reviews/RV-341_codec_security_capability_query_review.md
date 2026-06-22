# RV-341 Codec Security Capability Query Review

- Review ID: `RV-341`
- Date: `2026-05-07`
- Scope: codec security capability query for current default and optional CRC32 policy.

## Findings

- `rsrx_codec_security_capabilities_t` now exposes current security-field capability flags separately from per-wire-profile frame sizing.
- `rsrx_codec_get_security_capabilities()` reports default CRC absence, optional CRC32 availability, and MAC/timestamp unavailability.
- `TC-CODEC-030` verifies the capability query so deployment policy code can distinguish default skeleton absence from optional CRC32 support without inferring it from documentation alone.

## Residual

- This does not select CRC32 as the default deployment policy.
- MAC, timestamp, and PDU-level parity remain future codec-security implementation work.
