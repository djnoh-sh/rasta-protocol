# Baseline Fetch Success Evidence

## Document Control

- Report ID: `EVID-CI-BLRUN-20260420-24662424670`
- Status: `Complete`
- Execution Date: `2026-04-20`
- Workflow: `.github/workflows/sil4-ci.yml`
- Commit ID: `499bf79e37d5184bab7d1617d16b565592702f28`
- PR or Branch: `sil4-evidence-baseline`

## Purpose

이 문서는 `sil4-ci` workflow가 실제 PR run에서 이전 `sil4-ci-logs` artifact를 baseline으로 fetch하고 `delta-aware` PR annotation을 생성했음을 기록한다.

## Runtime Context

| Field | Value |
| --- | --- |
| Event Type | `pull_request` |
| Workflow Run ID | `24662424670` |
| Job Name | `sil4-verify` |
| Trigger Ref | `refs/pull/1/merge` |
| Artifact Name | `sil4-ci-logs` |

## Baseline Resolution Result

| Field | Value |
| --- | --- |
| Baseline Found | `true` |
| Source Type | `same-pr` |
| Source Run ID | `24661353609` |
| Materialized File | `sil4-ci-logs2/baseline_summary.env` |

## Verification Snapshot

| Field | Value |
| --- | --- |
| Current Summary Present | `yes` |
| Baseline Summary Present | `yes` |
| PR Annotation Present | `yes` |
| Annotation Baseline Mode | `delta-aware` |
| Annotation Delta Status | `baseline summary loaded` |

## Artifact References

- workflow URL: `https://github.com/djnoh-sh/rasta-protocol/actions/runs/24662424670`
- artifact reference: `https://github.com/djnoh-sh/rasta-protocol/actions/runs/24662424670/artifacts`
- local reviewed artifact dir: `sil4-ci-logs2/`
- `summary.env`: `sil4-ci-logs2/summary.env`
- `baseline_artifact.zip`: `sil4-ci-logs2/baseline_artifact.zip`
- `baseline_summary.env`: `sil4-ci-logs2/baseline_summary.env`
- `pr_annotation.md`: `sil4-ci-logs2/pr_annotation.md`

## Required Log References

1. `Resolve Baseline Artifact`: `SIL4_CI_run_24662424670_Resolve_Baseline_Artifact`
2. `Download Baseline Artifact`: `SIL4_CI_run_24662424670_Download_Baseline_Artifact`
3. `Materialize Baseline Summary`: `SIL4_CI_run_24662424670_Materialize_Baseline_Summary`
4. `Render PR Annotation`: `SIL4_CI_run_24662424670_Render_PR_Annotation`

## Confirmation Checklist

- [x] prior artifact identified
- [x] baseline artifact downloaded
- [x] `baseline_summary.env` materialized
- [x] annotation mode recorded as `delta-aware`
- [x] PR annotation generated as `pr_annotation.md`

## Conclusion

- Runtime Evidence Status: `Accepted`
- Follow-up Review: `RV-BLRUN-20260420-24662424670`
