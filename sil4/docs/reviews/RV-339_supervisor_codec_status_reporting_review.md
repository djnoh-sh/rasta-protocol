# RV-339 Supervisor Codec Status Reporting Review

- Review ID: `RV-339`
- Date: `2026-05-07`
- Scope: transport supervisor reporting of the last codec decode status.

## Findings

- `rsrx_transport_supervisor_report_t` now exposes `eLastCodecStatus`.
- `rsrx_transport_supervisor_process_frame()` stores the codec decode result before classifying success or decode failure.
- `TC-SUP-001`, `TC-SUP-003`, `TC-SUP-004`, and `TC-SUP-070` cover success, decode failure, unsupported message, and init-baseline status retention.

## Residual

- This preserves codec-specific status at the supervisor boundary, but does not yet create distinct supervisor decisions for CRC mismatch/truncated status.
- MAC/timestamp semantics and end-to-end CRC32 policy selection remain future codec-security policy growth.
