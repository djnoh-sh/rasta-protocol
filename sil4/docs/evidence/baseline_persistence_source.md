# Baseline Persistence Source

## Document Control

- Document ID: `EVID-028`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 delta-aware PR annotation이 참조할 baseline summary를 어디에서 가져오고 어떤 우선순위로 선택할지 정의한다.

목적은 다음과 같다.

1. `baseline_summary.env`의 source를 ad-hoc 방식이 아니라 통제된 규칙으로 고정한다.
2. helper, workflow, artifact retention 사이의 역할을 분리한다.
3. baseline 부재 시 snapshot-only fallback이 언제 허용되는지 명확히 한다.

## Scope

- 포함:
  - baseline source 우선순위
  - persistence location 후보
  - fetch/store responsibility
  - fallback rule
- 제외:
  - GitHub API implementation detail
  - external database 도입
  - non-`sil4` workflow integration

## Baseline Source Priority

delta-aware annotation은 아래 순서로 baseline을 선택한다.

1. 같은 PR의 가장 최근 successful `sil4-ci` artifact summary
2. 대상 branch의 가장 최근 successful `main` baseline summary
3. 둘 다 없으면 snapshot-only fallback

## Current Decision

### Primary Source

- primary baseline source는 `same PR previous successful run`이다.
- 이유:
  - 가장 직접적인 delta 비교가 가능하다.
  - unrelated branch drift를 줄인다.
  - reviewer가 PR 내부 변화만 판단할 수 있다.

### Secondary Source

- secondary baseline source는 `main branch last successful baseline`이다.
- 이유:
  - 신규 PR 첫 실행에서도 최소 기준점을 제공한다.
  - branch 생성 이후 drift를 감지할 수 있다.

### Fallback

- 위 두 source 모두 없으면 `snapshot-only`를 허용한다.
- 이 경우 annotation에는 반드시 다음이 드러나야 한다.
  - baseline unavailable
  - snapshot-only mode
  - delta not applied

## Persistence Format

- persisted artifact format은 `summary.env`를 그대로 사용한다.
- rename 없이 복제본 이름만 `baseline_summary.env`로 맞춘다.
- 이유:
  - helper가 parser를 따로 두지 않아도 된다.
  - summary generator와 baseline consumer 간 포맷 drift를 줄인다.

## Storage Boundary

- CI artifact boundary를 persistence unit으로 사용한다.
- 현재 단계에서는 아래를 기본 단위로 본다.
  - workflow artifact `sil4-ci-logs`
  - included file `summary.env`

장기적으로는 PR comment body나 branch artifact catalog를 reference index로 사용할 수 있지만, source of truth는 artifact file로 둔다.

## Responsibility Split

### Helper

- helper는 `baseline_summary.env`가 이미 준비돼 있다고 가정한다.
- helper는 source selection을 하지 않는다.
- helper 책임은 아래로 제한한다.
  - baseline file 존재 여부 확인
  - delta 계산
  - annotation rendering

### Workflow

- workflow는 baseline source를 결정하고, 선택된 baseline을 `baseline_summary.env`로 materialize 한다.
- workflow는 source selection 실패 시 snapshot-only로 계속 진행한다.

### Evidence Owner

- evidence owner는 baseline source policy 변경 시 본 문서와 `ci_execution_linkage.md`, `pr_annotation_strategy.md`를 동시에 갱신한다.

## Fetch/Store Policy

1. current run이 끝나면 `summary.env`를 artifact에 업로드한다.
2. subsequent run은 source priority에 따라 prior artifact를 조회한다.
3. 조회 성공 시 해당 file을 `/tmp/rsrx-ci-logs/baseline_summary.env`로 저장한다.
4. 조회 실패 시 helper는 snapshot-only로 동작한다.

## Failure Handling

다음 경우는 hard failure로 보지 않는다.

- baseline artifact not found
- baseline artifact download unavailable
- baseline file parse target key missing

이 경우 정책은 `delta unavailable, snapshot-only fallback`이다.

다음 경우는 review note를 남긴다.

- baseline source가 반복적으로 unavailable
- main baseline과 PR baseline 사이 source mismatch가 잦음

## Security and Integrity Notes

- baseline file은 trusted CI artifact에서만 가져온다.
- PR branch workspace 내부 파일을 baseline source로 직접 사용하지 않는다.
- manual override가 필요하면 review record에 justification을 남긴다.

## Exit Criteria

- baseline source priority가 문서로 고정됐다.
- helper/workflow responsibility가 분리됐다.
- fallback rule이 명시됐다.
- roadmap와 evidence index가 본 결정을 반영한다.

## Follow-up Actions

1. first workflow baseline fetch success evidence 생성
2. PR previous-success lookup failure path 관찰
3. artifact retention 기간과 baseline availability 관계 점검
