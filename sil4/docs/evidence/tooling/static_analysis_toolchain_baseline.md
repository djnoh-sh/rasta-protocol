# Static Analysis Toolchain Baseline

## Document Control

- Document ID: `EVID-004`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-04-29`

## Purpose

이 문서는 `sil4/` 코드베이스에 대해 반복 실행 가능한 baseline 정적분석 도구 조합을 고정한다.

## Baseline Toolchain

| Role | Tool | Version | Status | Notes |
| --- | --- | --- | --- | --- |
| Compiler Warning Gate | `gcc` | `13.3.0` | Baseline | `cmake --build /tmp/sil4-build -j4` 경고 0 기준 |
| Secondary Compiler | `clang` | `18.1.3` | Reference | 현재 baseline gate에는 미포함 |
| Static Analyzer | `cppcheck` | `2.13.0` | Baseline | `warning,style,performance,portability` profile 적용 |

## Selected Rule Profile

- Compiler:
  - `-Wall -Wextra -Werror`
  - optional evaluation profile: `-Wconversion -Wsign-conversion` via `-DRSRX_ENABLE_STRICT_WARNING_HARDENING=ON`
- `cppcheck`:
  - `--enable=warning,style,performance,portability`
  - `--std=c11`
  - `--force`
  - `--inline-suppr`

## Current Decision

현재 baseline 정적분석 체인은 다음 두 단계로 정의한다.

1. compiler warning gate
2. `cppcheck` baseline scan

이 조합은 다음 이유로 채택한다.

- 현재 환경에 이미 설치되어 있다.
- 재현 가능하다.
- `sil4/` 범위를 빠르게 전수 확인할 수 있다.
- 전용 상용 MISRA 도구 도입 전까지 반복 가능한 최소 기준을 제공한다.

## Known Limitations

- `cppcheck`는 MISRA 완전 대체가 아니다.
- 현재 profile은 style/performance warning까지 포함하므로 false positive 검토가 필요하다.
- toolchain path와 CI integration은 아직 고정되지 않았다.

## Next Upgrade Path

1. CI에서 동일 스크립트 실행
2. `clang` 기반 보조 경고 게이트 검토
3. 전용 MISRA analyzer 도입 후 baseline 재정의

## 2026-04-29 Hardening Evaluation Note

- external V&V follow-up에 따라 `Makefile`이 아니라 CMake 기준 warning-hardening evaluation을 수행한다.
- baseline gate는 여전히 `-Wall -Wextra -Werror`다.
- stricter conversion/sign profile은 `RSRX_ENABLE_STRICT_WARNING_HARDENING` option으로 분리한다.
- current GCC baseline environment에서 strict profile build cleanliness를 별도 evidence report로 기록한다.
