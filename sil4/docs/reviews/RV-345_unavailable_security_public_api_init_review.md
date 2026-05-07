# RV-345 Unavailable Security Public API Init Review

- Review ID: `RV-345`
- Date: `2026-05-07`
- Scope: public API session initialization behavior for unavailable MAC/timestamp deployment policies.

## Findings

- `TC-API-015` verifies that `uRequireMac=1` is rejected by `rsrx_session_init()`.
- `TC-API-015` also verifies that `uRequireTimestamp=1` is rejected by `rsrx_session_init()`.
- Rejected sessions remain outside initialized state, preserving the startup gate as a hard failure rather than a silent downgrade.

## Residual

- This confirms public API enforcement for unsupported MAC/timestamp policies.
- Actual MAC, timestamp, and PDU parity remain future codec-security implementation work.
