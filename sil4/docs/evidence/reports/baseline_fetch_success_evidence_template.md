# Baseline Fetch Success Evidence Template

## Document Control

- Report ID: `EVID-CI-RUN-TBD`
- Status: `Draft`
- Execution Date: `YYYY-MM-DD`
- Workflow: `.github/workflows/sil4-ci.yml`
- Commit ID: `TBD`
- PR or Branch: `TBD`

## Purpose

이 템플릿은 `sil4-ci` workflow가 baseline artifact fetch를 실제로 성공했을 때 그 증빙을 기록하기 위한 것이다.

## Runtime Context

| Field | Value |
| --- | --- |
| Event Type | `pull_request` / `push` |
| Workflow Run ID | `TBD` |
| Job Name | `sil4-verify` |
| Trigger Ref | `TBD` |
| Artifact Name | `sil4-ci-logs` |

## Baseline Resolution Result

| Field | Value |
| --- | --- |
| Baseline Found | `true` |
| Source Type | `same-pr` / `main-branch` |
| Source Run ID | `TBD` |
| Materialized File | `/tmp/rsrx-ci-logs/baseline_summary.env` |

## Verification Snapshot

아래 항목을 actual run artifact에서 채운다.

| Field | Value |
| --- | --- |
| Current Summary Present | `yes/no` |
| Baseline Summary Present | `yes/no` |
| PR Annotation Present | `yes/no` |
| Annotation Baseline Mode | `delta-aware` |
| Annotation Delta Status | `baseline summary loaded` |

## Artifact Evidence

최소 아래 파일 또는 로그 단편을 참조한다.

- `summary.env`
- `baseline_artifact.zip`
- `baseline_summary.env`
- `pr_annotation.md`
- workflow step logs for:
  - `Resolve Baseline Artifact`
  - `Download Baseline Artifact`
  - `Materialize Baseline Summary`

## Expected Confirmation Points

1. workflow가 prior artifact를 식별했다.
2. artifact zip download가 성공했다.
3. `summary.env` 추출이 성공했다.
4. `render_pr_annotation.sh`가 `Baseline Mode: delta-aware`로 동작했다.
5. sticky PR comment 또는 step summary에 delta-aware annotation이 게시됐다.

## Failure Notes

실패가 있었다면 아래를 기록한다.

- source resolution failure
- artifact download failure
- archive extraction failure
- malformed `summary.env`
- snapshot-only fallback 여부

## Review Linkage

- Required Review Record: `RV-TBD`
- Related Policy:
  - `baseline_persistence_source.md`
  - `ci_execution_linkage.md`
  - `pr_annotation_strategy.md`
  - `delta_based_annotation_policy.md`

## Conclusion

- Baseline fetch runtime evidence status:
  - `Pass` / `Partial` / `Fail`
- Follow-up:
  - `TBD`
