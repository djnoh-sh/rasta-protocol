# Tool-Specific MISRA Mapping Baseline

## Document Control

- Document ID: `EVID-019`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 현재 `sil4/` 재구현 코드베이스에서 사용하는 정적분석 도구 출력과 MISRA subset severity baseline 사이의 연결 규칙을 정의한다.

목적은 다음과 같다.

1. tool output을 바로 `MISRA-S1`~`MISRA-S6` subset으로 분류한다.
2. deviation log와 review record가 tool output을 동일한 기준으로 해석하게 만든다.
3. 향후 전용 MISRA analyzer를 도입할 때 rule ID level mapping으로 확장할 기반을 만든다.

## Scope

- 포함:
  - compiler warning gate
  - `cppcheck` baseline categories
  - future MISRA analyzer placeholder mapping
- 제외:
  - vendor-specific commercial rule catalog full import
  - non-`sil4/` prototype code

## Inputs

- subset severity baseline:
  - `misra_subset_severity.md`
- general severity baseline:
  - `severity_mapping.md`
- toolchain baseline:
  - `tooling/static_analysis_toolchain_baseline.md`
- deviation control:
  - `misra_deviation_log.md`

## Mapping Rules

### Rule 1

도구가 직접 MISRA rule ID를 제공하지 않는 경우에도, finding category와 code context를 기준으로 subset ID를 먼저 부여한다.

### Rule 2

subset ID가 정해지면 severity는 기본적으로 `misra_subset_severity.md`의 default severity를 따른다.

### Rule 3

tool output severity와 subset severity가 다를 경우, safety gate 판단은 더 높은 쪽을 따른다.

### Rule 4

style-only finding이라도 실제로 state, timer, redundancy, transport feedback 경계에 영향을 주면 `MISRA-S6`로 두지 않고 상위 subset으로 승격한다.

## Current Tool Mapping Matrix

| Tool Source | Finding Pattern | Primary Subset | Default Severity | Notes |
| --- | --- | --- | --- | --- |
| `gcc` / `clang` warning | uninitialized variable, maybe-uninitialized | `MISRA-S2` | `High` | control/status/timer variable은 safety path로 간주 |
| `gcc` / `clang` warning | incompatible pointer type, enum/int mismatch, narrowing conversion | `MISRA-S4` | `High` | API/port boundary는 즉시 review 필요 |
| `gcc` / `clang` warning | switch/enum coverage, implicit fall-through without intent | `MISRA-S3` | `High` | state machine/supervisor 경계에서 우선 차단 |
| `cppcheck` | `uninitvar`, `unassignedVariable` | `MISRA-S2` | `High` | initialization gate |
| `cppcheck` | `nullPointer`, `memleak`, `bufferAccessOutOfBounds` 계열 | `MISRA-S1` | `Critical` | memory safety gate |
| `cppcheck` | `duplicateBreak`, invalid control path, missing error propagation | `MISRA-S3` or `MISRA-S5` | `High` or `Medium` | state/timer path면 `MISRA-S3` 우선 |
| `cppcheck` | `constParameter`, `redundantAssignment`, `variableScope` | `MISRA-S6` | `Low` | style-only로 남을 때만 허용 |
| `cppcheck` | unchecked return, ignored status on platform/transport API | `MISRA-S5` | `Medium` | safety path면 `High`로 승격 가능 |
| future MISRA analyzer | explicit memory/lifetime rule | `MISRA-S1` | `Critical` | vendor rule ID 추가 예정 |
| future MISRA analyzer | explicit type/interface rule | `MISRA-S4` | `High` | vendor rule ID 추가 예정 |

## Classification Procedure

1. finding source를 식별한다.
2. finding pattern을 현재 mapping matrix에 대입한다.
3. context가 state machine, supervisor, timer, transport, redundancy, API safety path인지 확인한다.
4. 필요 시 subset을 상향 조정한다.
5. deviation 또는 review record에는 `Rule ID`, `Subset ID`, `Severity`를 함께 기록한다.

## Deviation Log Use

편차가 승인 또는 추적 대상으로 남는 경우 최소 아래를 기록한다.

- `Rule ID`
- `Subset ID`
- `Severity`
- `Justification`
- `Risk Assessment`
- `Mitigation`

현재 `misra_deviation_log.md`는 이 구조를 이미 수용하도록 `Subset ID` 열을 포함한다.

## Current Limits

- 현재 baseline은 `cppcheck`와 compiler warning 중심이다.
- commercial MISRA analyzer rule number는 아직 연결하지 않았다.
- subset assignment는 일부 case에서 reviewer judgement를 요구한다.

## Follow-up Actions

1. second tool 후보를 정하고 동일 matrix에 추가
2. commercial MISRA analyzer 도입 시 vendor rule ID level mapping 확장
3. CI summary에 subset bucket 집계 추가
