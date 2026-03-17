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

## Rationale

- 로컬과 CI 사이 절차 차이를 줄인다.
- evidence 보고서에 적는 실행 절차를 workflow와 직접 연결한다.
- 기존 저장소 전체 CMake workflow와 분리해 `sil4/` 변경만 독립적으로 검증한다.

## Residual Limits

- 현재 workflow는 Linux 단일 플랫폼만 사용한다.
- 결과 요약 업로드, artifact 보관, PR annotation은 아직 없다.
- MISRA 전용 analyzer는 CI에 포함되지 않는다.

## Follow-up Actions

1. CI artifact upload 추가
2. severity mapping 문서와 workflow fail policy 연결
3. second-tool analyzer 도입 여부 결정
