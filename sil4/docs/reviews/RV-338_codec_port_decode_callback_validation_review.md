# RV-338 Codec Port Decode Callback Validation Review

- Review ID: `RV-338`
- Date: `2026-05-07`
- Scope: codec port completeness validation across session configuration and transport adapter initialization.

## Findings

- `rsrx_validate_session_config()` now rejects codec ports missing either `pfEncode` or `pfDecode`.
- `rsrx_transport_adapter_init()` now applies the same encode/decode completeness rule before storing the selected codec port.
- `TC-CFG-003` covers the session startup gate, and `TC-PA-011` covers the transport adapter initialization gate.

## Residual

- This does not change the default codec selection policy; callers still explicitly select the default skeleton port or CRC32 port.
- End-to-end CRC32 adoption remains future policy work, but incomplete selected codec ports are no longer accepted at startup boundaries.
