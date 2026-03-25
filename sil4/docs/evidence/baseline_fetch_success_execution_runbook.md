# Baseline Fetch Success Execution Runbook

## Document Control

- Document ID: `EVID-CI-051`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-25`

## Purpose

이 문서는 `EVS-001`, `EVS-002`를 실제 `sil4-ci` successful run으로 채울 때 필요한 정확한 실행/채움 순서를 고정한다.

목표는 first successful baseline fetch run이 나왔을 때 추가 해석 없이

1. runtime report를 채우고
2. runtime review를 작성하고
3. tracker와 audit trail을 갱신하는 것

이다.

## Preconditions

다음이 모두 성립해야 한다.

1. event type이 `pull_request`
2. current run에서 `Resolve Baseline Artifact` output `found == true`
3. `Download Baseline Artifact` 성공
4. `Materialize Baseline Summary` 성공
5. `Render PR Annotation`가 `delta-aware` mode로 수행됨

## Evidence Sources

actual evidence는 아래 source에서만 채운다.

- workflow run page
- `sil4-ci-logs` artifact
- `/tmp/rsrx-ci-logs/summary.env`
- `/tmp/rsrx-ci-logs/baseline_summary.env`
- `/tmp/rsrx-ci-logs/pr_annotation.md`
- workflow logs of:
  - `Resolve Baseline Artifact`
  - `Download Baseline Artifact`
  - `Materialize Baseline Summary`
  - `Render PR Annotation`

## Fill-In Procedure

### 1. Runtime Report

target artifact:

- `reports/baseline_fetch_success_evidence_first_run_stub.md`

fill:

1. `Execution Date`
2. `Commit ID`
3. `PR or Branch`
4. `Workflow Run ID`
5. `Trigger Ref`
6. `Baseline Found`
7. `Source Type`
8. `Source Run ID`
9. `Current Summary Present`
10. `Baseline Summary Present`
11. `PR Annotation Present`
12. `Annotation Baseline Mode`
13. `Annotation Delta Status`
14. workflow/artifact/file references
15. exact log snippet or line reference

### 2. Runtime Review

target artifact:

- `reviews/RV-TBD_baseline_fetch_success_runtime_review_stub.md`

verify:

1. workflow run id / trigger ref / commit id consistency
2. baseline source and `baseline_summary.env` consistency
3. required log references present
4. annotation mode is `delta-aware`
5. evidence index / tracker / audit trail updated

### 3. Tracker Update

target artifact:

- `first_actual_vendor_evidence_set_execution_tracker.md`

update:

- `EVS-001` -> `Closed`
- `EVS-002` -> `Closed`
- execution note에 date/reference 추가

### 4. Audit Trail Update

target artifact:

- `audit_trail_closeout.md`

add:

1. actual runtime report link
2. actual runtime review link
3. tracker close status reference

## Expected Completed Set

이 runbook가 완료되면 최소 아래가 닫혀 있어야 한다.

1. `EVS-001`
2. `EVS-002`
3. tracker baseline fetch track
4. audit trail baseline fetch landing zone link

## Notes

- `push` event run은 baseline fetch success evidence의 first actual run으로 취급하지 않는다.
- `snapshot-only` annotation이면 본 runbook 완료 조건을 만족하지 않는다.
- 실제 artifact path나 workflow step 이름이 바뀌면 본 문서부터 갱신한다.
