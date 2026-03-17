# PR Annotation Strategy

## Document Control

- Document ID: `EVID-023`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 `sil4` CI 실행 결과를 pull request 수준에서 어떻게 annotation할지 기준을 정의한다.

목적은 다음과 같다.

1. 단순 pass/fail을 넘어 reviewer가 바로 판단할 수 있는 핵심 신호를 제공한다.
2. severity bucket, MISRA subset bucket, static analysis finding을 annotation policy와 연결한다.
3. PR noise를 통제하면서 safety-relevant signal만 표면화한다.

## Scope

- 포함:
  - GitHub PR comment/summary/annotation 정책
  - severity/subset bucket 기반 표시 규칙
  - build/test/static analysis failure reporting 기준
- 제외:
  - external dashboard
  - repository-wide non-`sil4` workflow

## Current Baseline Inputs

- CI summary:
  - `sil4/tools/run_ci_verification.sh`
- workflow:
  - `.github/workflows/sil4-ci.yml`
- severity mapping:
  - `sil4/docs/evidence/severity_mapping.md`
- subset mapping:
  - `sil4/docs/evidence/tool_specific_misra_mapping.md`

## Annotation Levels

| Level | Trigger | Intended Surface |
| --- | --- | --- |
| `Blocker` | configure/build/test failure, `Critical` finding, unapproved `High` finding | PR failure + explicit annotation |
| `Review Required` | `Medium` finding, deviation opened, unexpected bucket increase | PR summary + reviewer attention note |
| `Informational` | all-zero clean run, stable bucket summary, expected low-risk cleanup | PR summary only |

## Policy Rules

### Rule 1

configure/build/test 단계가 실패하면 반드시 `Blocker`로 기록한다.

### Rule 2

`summary.env`의 severity bucket에서 `Critical > 0`이면 `Blocker`다.

### Rule 3

`High > 0`이면 기본적으로 `Blocker`다. 다만 승인된 deviation과 정확히 연결된 경우에만 `Review Required`로 완화 가능하다.

### Rule 4

`Medium > 0`이고 `High/Critical == 0`이면 `Review Required`다.

### Rule 5

`Low` 또는 `Info`만 존재하면 `Informational`이다.

### Rule 6

subset bucket의 unexpected increase는 severity bucket과 별도로 review note를 남긴다. 특히 아래는 우선순위를 높인다.

- `MISRA-S1`
- `MISRA-S2`
- `MISRA-S3`
- `MISRA-S4`

## Recommended Output Format

### Summary Block

- overall result
- configure/build/test/static analysis status
- severity bucket table
- subset bucket table
- artifact link

### Annotation Block

- `Blocker`가 있으면 top-level failure reason 한 줄
- `Review Required`가 있으면 reviewer action 한 줄
- `Informational`만 있으면 clean baseline confirmation 한 줄

## Current Decision

1. `summary.env` 기반 helper script를 통해 PR annotation markdown을 생성한다.
2. workflow는 `pull_request` 이벤트에서 helper를 실행하고 step summary에 그 결과를 게시한다.
3. workflow는 GitHub API를 사용해 marker 기반 sticky PR comment를 create/update 한다.
4. helper는 optional baseline file `/tmp/rsrx-ci-logs/baseline_summary.env`를 읽어 delta-aware annotation을 계산할 수 있다.
5. baseline source 우선순위는 `baseline_persistence_source.md`를 따른다.
6. workflow는 prior artifact fetch를 시도하고, 성공 시 baseline file을 materialize 한다.

## Minimal Implementation Path

### Step A

- `summary.env`를 읽어 level(`Blocker/Review Required/Informational`)을 계산한다.

### Step B

- `pull_request` 이벤트에서 `GITHUB_STEP_SUMMARY`와 별도로 short annotation comment를 생성한다.

### Step C

- GitHub API로 marker 기반 sticky comment를 create/update 한다.

### Step D

- noise가 크면 `Blocker`와 `Review Required`만 annotation하고 `Informational`은 summary에만 남기는 최적화 여부를 검토한다.

## Residual Limits

- sticky PR comment는 구현됐고 helper는 optional baseline file이 주어지면 delta-aware annotation을 계산한다.
- baseline persistence source policy와 workflow fetch step은 구현됐지만 runtime success evidence는 아직 제한적이다.
- exact vendor rule matrix가 없어 vendor-level annotation은 불가능하다.
- trend comparison은 baseline file이 주어질 때만 동작한다.

## Follow-up Actions

1. `summary.env` 기반 decision logic regression 검증
2. PR noise threshold 정의
3. first workflow baseline fetch success evidence 확보
