# CI Execution Linkage

## Document Control

- Document ID: `EVID-016`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 `sil4/` 재구현 코드베이스의 로컬 검증 절차와 CI 실행 절차를 동일한 스크립트로 연결하기 위한 기준을 정의한다.

## Scope

- 대상 코드:
  - `sil4/include`
  - `sil4/src`
  - `sil4/tests/unit`
  - `sil4/tests/integration`
- 대상 산출물:
  - `sil4/tools/run_ci_verification.sh`
  - `.github/workflows/sil4-ci.yml`

## Execution Chain

1. `bash sil4/tools/run_ci_verification.sh`
2. configure
3. build
4. unit/integration executable run
5. `cppcheck` baseline

CI workflow는 위 스크립트를 그대로 호출한다.

## Current CI Gate

| Stage | Command Source | Failure Condition |
| --- | --- | --- |
| Configure | `cmake -S sil4 -B <build>` | configure non-zero exit |
| Build | `cmake --build <build> -j4` | compiler warning/error, build non-zero exit |
| Test | named test executables | any executable non-zero exit |
| Static Analysis | `cppcheck --enable=warning,style,performance,portability ...` | non-zero exit |

현재 severity 해석 기준은 `sil4/docs/evidence/severity_mapping.md`를 참조한다.

## CI Outputs

- workflow는 `/tmp/rsrx-ci-logs/summary.md`를 step summary로 게시한다.
- summary는 severity bucket과 MISRA subset bucket 집계를 포함한다.
- PR helper는 optional baseline file `/tmp/rsrx-ci-logs/baseline_summary.env`가 존재하면 delta-aware annotation을 계산한다.
- workflow는 아래 로그를 artifact `sil4-ci-logs`로 업로드한다.
  - `configure.log`
  - `build.log`
  - `tests.log`
  - `cppcheck.log`
  - `summary.md`
  - `summary.env`
  - `baseline_summary.env` (optional)

## Rationale

- 로컬과 CI 사이 절차 차이를 줄인다.
- evidence 보고서에 적는 실행 절차를 workflow와 직접 연결한다.
- 기존 저장소 전체 CMake workflow와 분리해 `sil4/` 변경만 독립적으로 검증한다.

## Residual Limits

- 현재 workflow는 Linux 단일 플랫폼만 사용한다.
- PR annotation helper와 sticky PR comment publication은 존재하고, helper는 optional baseline file이 있으면 delta-aware annotation을 계산한다.
- baseline persistence source policy는 `baseline_persistence_source.md`로 고정됐지만, workflow fetch step은 아직 구현되지 않았다.
- `ctest` registration은 아직 primary path가 아니다.
- MISRA 전용 analyzer는 CI에 포함되지 않는다.

## Follow-up Actions

1. severity mapping 문서와 workflow fail policy 연결 강화
2. second-tool analyzer 도입 여부 결정
3. `ctest` 전환 조건 재검토
4. baseline artifact fetch step 구현
