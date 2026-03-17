# CTest Registration Strategy

## Document Control

- Document ID: `EVID-022`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 현재 `sil4/` 검증 체인이 executable direct-run 방식을 사용하는 이유와, 향후 `ctest` 등록을 어떤 조건에서 도입할지 정의한다.

## Current State

- `sil4/CMakeLists.txt`는 unit/integration test executable을 생성한다.
- 현재 CI와 local verification은 아래 스크립트로 직접 실행한다.
  - `sil4/tools/run_ci_verification.sh`
- `ctest --test-dir <build>` 실행 시 현재 등록된 테스트가 없어 `No tests were found` 상태다.

## Why Direct-Run Is Kept For Now

1. 현재 각 test executable은 자체 return code로 pass/fail을 명확히 제공한다.
2. 실행 순서가 고정돼 있어 CI summary와 evidence report 작성이 단순하다.
3. 테스트 수가 아직 적고, per-test metadata보다 baseline reproducibility가 더 중요하다.
4. direct-run 방식은 로컬/CI/second-tool baseline에서 동일하게 재사용 가능하다.

## Limitations Of Current State

- `ctest` summary를 바로 활용할 수 없다.
- test label, timeout, fixture, partial rerun 같은 메타데이터를 CMake 레벨에서 다루지 못한다.
- audit 설명 시 “왜 CTest를 쓰지 않는가”를 별도 문서로 설명해야 한다.

## Decision Baseline

현재 단계에서는 `direct-run`을 baseline으로 유지한다.

`ctest` 등록은 아래 조건이 충족될 때 도입 대상으로 승격한다.

1. test executable naming과 responsibility가 안정적일 것
2. timeout, label, fixture 같은 metadata 사용 이점이 실제로 필요할 것
3. CI summary와 evidence 절차가 `ctest` output으로도 동일하게 유지될 것
4. direct-run 대비 재현성이 약해지지 않을 것

## Recommended Migration Path

### Step 1

- 현행 direct-run 유지
- evidence/CI summary는 direct-run 기준으로 계속 관리

### Step 2

- `enable_testing()`와 `add_test()`를 도입하되, 기존 direct-run은 병행 유지
- `ctest` output을 reference-only로 수집

### Step 3

- `ctest` metadata가 timeout/label grouping/selection에 실질 이점을 주는지 검토
- audit 문서에서 `direct-run`과 `ctest` 결과의 동등성 확인

### Step 4

- `ctest`를 primary path로 승격할지 결정
- 승격 시 direct-run path는 fallback 또는 debug path로 유지 여부를 결정

## Exit Criteria For Strategy Closure

- `enable_testing()` 도입
- 모든 test executable에 `add_test()` 등록
- CI summary와 evidence report가 `ctest` 출력과 연결됨
- direct-run 대비 동등성 검토 review 완료

## Current Recommendation

- 지금 당장은 `ctest` 도입보다 direct-run 유지가 더 합리적이다.
- 이유는 현재 과제의 병목이 test registration 자체가 아니라 protocol/evidence completion이기 때문이다.
- 따라서 `ctest`는 `supporting improvement`로 다루고, P5/P4의 핵심 잔여 리스크보다 우선하지 않는다.

## Follow-up Actions

1. `enable_testing()` 도입 가능성 검토
2. `add_test()` boilerplate 초안 작성 여부 판단
3. CI summary가 direct-run/ctest 양쪽을 모두 수용할 수 있는지 검토
