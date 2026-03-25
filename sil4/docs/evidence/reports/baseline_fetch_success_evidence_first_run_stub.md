# Baseline Fetch Success Evidence - First Run Stub

## Document Control

- Report ID: `EVID-CI-RUN-001`
- Status: `Stub`
- Execution Date: `TBD`
- Workflow: `.github/workflows/sil4-ci.yml`
- Commit ID: `TBD`
- PR or Branch: `TBD`

## Purpose

이 문서는 `EVS-001`을 실제 workflow run 결과로 즉시 채우기 위한 first-run stub다.

현재는 runtime 값이 비어 있으며, 첫 baseline fetch success run이 나오면 아래 항목을 그대로 채운다.

## Runtime Context

| Field | Value |
| --- | --- |
| Event Type | `TBD` |
| Workflow Run ID | `TBD` |
| Job Name | `sil4-verify` |
| Trigger Ref | `TBD` |
| Artifact Name | `sil4-ci-logs` |

## Baseline Resolution Result

| Field | Value |
| --- | --- |
| Baseline Found | `TBD` |
| Source Type | `TBD` |
| Source Run ID | `TBD` |
| Materialized File | `/tmp/rsrx-ci-logs/baseline_summary.env` |

## Verification Snapshot

| Field | Value |
| --- | --- |
| Current Summary Present | `TBD` |
| Baseline Summary Present | `TBD` |
| PR Annotation Present | `TBD` |
| Annotation Baseline Mode | `TBD` |
| Annotation Delta Status | `TBD` |

## Artifact References

- workflow URL: `TBD`
- artifact URL or run artifact reference: `TBD`
- `summary.env`: `TBD`
- `baseline_artifact.zip`: `TBD`
- `baseline_summary.env`: `TBD`
- `pr_annotation.md`: `TBD`

## Required Log Snippets

아래 step의 exact log snippet 또는 line reference를 채운다.

1. `Resolve Baseline Artifact`
2. `Download Baseline Artifact`
3. `Materialize Baseline Summary`
4. `Render PR Annotation`

## Expected Confirmation Checklist

- [ ] prior artifact 식별 성공
- [ ] baseline artifact download 성공
- [ ] `baseline_summary.env` materialize 성공
- [ ] annotation mode가 `delta-aware`로 기록됨
- [ ] step summary 또는 PR comment에 annotation 게시됨

## Conclusion

- Runtime Evidence Status: `TBD`
- Follow-up Review: `RV-TBD`
