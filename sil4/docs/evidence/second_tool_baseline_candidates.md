# Second-Tool Baseline Candidates

## Document Control

- Document ID: `EVID-020`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 현재 `gcc + cppcheck` baseline 이후에 추가할 second-tool 후보와 선택 기준을 정의한다.

목적은 다음과 같다.

1. second-tool 도입을 ad-hoc 판단이 아니라 evidence-driven decision으로 바꾼다.
2. MISRA subset severity baseline, tool-specific mapping baseline과 연결 가능한 후보만 남긴다.
3. CI와 로컬에서 재현 가능한 범위 안에서 우선순위를 정한다.

## Current Baseline Reference

현재 baseline toolchain은 다음과 같다.

1. compiler warning gate: `gcc`
2. static analyzer: `cppcheck`

현재 공백은 다음과 같다.

- compiler diversity가 부족하다.
- richer path-sensitive warning이 약하다.
- vendor-level MISRA rule mapping 입력이 아직 없다.

## Candidate Matrix

| Candidate ID | Tool | Role | Strength | Limitation | Baseline Recommendation |
| --- | --- | --- | --- | --- | --- |
| ST-001 | `clang` warning build | secondary compiler gate | compiler diversity 확보, enum/type/signature 문제 조기 검출 | MISRA direct mapping은 제한적 | `Adopt First` |
| ST-002 | `clang-tidy` | semantic/static check | modern C diagnostics, readability 제외 profile 구성 가능 | profile tuning 비용이 큼, false positive 관리 필요 | `Evaluate After ST-001` |
| ST-003 | commercial MISRA analyzer | rule-level MISRA evidence | vendor rule ID, deviation linkage, audit 설명력 우수 | 비용/도입 환경/CI 연계 부담 큼 | `Strategic Target` |
| ST-004 | `sparse` 또는 유사 lightweight checker | pointer/type qualifier 보조 점검 | 특정 type/annotation 오류 탐지에 유리 | 현재 코드/환경 적합성 미확인 | `Optional` |

## Selection Criteria

### SC-1 Reproducibility

- 로컬과 CI에서 동일 명령으로 반복 실행 가능해야 한다.
- 설치/실행 절차가 evidence 문서에 기록 가능해야 한다.

### SC-2 Signal Quality

- 새로 얻는 finding이 `gcc`/`cppcheck`와 중복만 되어서는 안 된다.
- 최소 하나 이상의 공백 영역을 메워야 한다.
  - compiler diversity
  - path-sensitive issue
  - MISRA rule-level mapping

### SC-3 Severity Mapping Fit

- finding을 `severity_mapping.md`와 `tool_specific_misra_mapping.md` 기준으로 분류할 수 있어야 한다.
- 필요 시 subset ID와 severity를 reviewer가 방어 가능하게 부여할 수 있어야 한다.

### SC-4 CI Cost

- baseline 단계에서는 CI 시간을 과도하게 늘리지 않아야 한다.
- 실행 실패 원인이 환경 문제인지 코드 문제인지 구분 가능해야 한다.

## Current Decision

1. immediate second-tool candidate는 `ST-001 clang warning build`로 둔다.
2. `clang-tidy`는 profile stabilization 전까지 baseline gate로 넣지 않는다.
3. commercial MISRA analyzer는 장기 목표로 유지하되, vendor procurement와 CI integration 조건이 준비되기 전에는 evidence placeholder로만 둔다.

## Proposed Rollout

### Step A

- `clang` warning-only build를 local reference run으로 추가
- 결과를 `SA-REP` 형식으로 별도 기록

### Step B

- false positive와 compiler-difference를 triage
- `severity_mapping.md`와 `tool_specific_misra_mapping.md`에 `clang` classification 예시 보강

### Step C

- CI optional job으로 연결
- 안정화 후 baseline gate 편입 여부 결정

## Exit Criteria For Candidate Closure

- second-tool 실행 스크립트 존재
- 적어도 1건의 baseline report와 review record 존재
- evidence index와 roadmap에 milestone 반영
- current risk `R-005`의 second-tool linkage 부분이 감소

## Residual Risks

- `clang`은 MISRA rule-level evidence를 직접 대체하지 않는다.
- `clang-tidy`는 profile 결정 전 과도한 style noise를 만들 수 있다.
- commercial analyzer는 비용과 라이선스, CI 연계가 가장 큰 장애다.

## Follow-up Actions

1. `clang` availability와 local command line baseline 확인
2. first second-tool report 초안 작성
3. CI optional path 여부 결정
