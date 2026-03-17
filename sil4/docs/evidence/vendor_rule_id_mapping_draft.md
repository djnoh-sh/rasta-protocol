# Vendor Rule ID Mapping Draft

## Document Control

- Document ID: `EVID-021`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 향후 commercial MISRA analyzer를 도입할 때 vendor rule ID를 현재 `MISRA-S1`~`MISRA-S6` subset 구조에 연결하기 위한 draft mapping 구조를 정의한다.

목적은 다음과 같다.

1. vendor tool 도입 시 기존 evidence 구조를 깨지 않고 rule-level detail을 추가한다.
2. rule ID, subset ID, severity, deviation record를 한 체계 안에서 연결한다.
3. 현재 baseline tool(`gcc`, `clang`, `cppcheck`) 결과와 future vendor rule ID를 병렬 관리할 수 있게 만든다.

## Scope

- 포함:
  - vendor rule family to subset mapping structure
  - deviation log linkage fields
  - CI/report integration placeholder
- 제외:
  - 특정 commercial tool 확정
  - 실제 vendor rule catalog 전체 import
  - 라이선스/조달 절차

## Current Baseline Dependencies

- subset severity baseline:
  - `misra_subset_severity.md`
- tool-specific mapping baseline:
  - `tool_specific_misra_mapping.md`
- severity baseline:
  - `severity_mapping.md`
- deviation control:
  - `misra_deviation_log.md`

## Mapping Model

### Level 1

- `Tool Source`
  - 예: `vendor-misra-tool`

### Level 2

- `Vendor Rule Family`
  - 예: memory, initialization, control-flow, type/interface, defensive checks

### Level 3

- `Vendor Rule ID`
  - 예: `VND-MEM-001`
  - 실제 rule number는 tool 확정 후 반영

### Level 4

- `Subset ID`
  - `MISRA-S1`~`MISRA-S6`

### Level 5

- `Severity`
  - `Critical/High/Medium/Low/Info`

## Draft Rule Family Mapping

| Vendor Rule Family | Example Rule Theme | Target Subset | Default Severity | Notes |
| --- | --- | --- | --- | --- |
| Memory | invalid pointer, buffer overrun, lifetime misuse | `MISRA-S1` | `Critical` | current baseline memory gate와 일치 |
| Initialization | uninitialized object, incomplete init, default state omission | `MISRA-S2` | `High` | timer/state/control variable 우선 |
| Control Flow | invalid switch handling, unsafe fall-through, unreachable recovery path | `MISRA-S3` | `High` | supervisor/state machine 경계 우선 |
| Type / Interface | incompatible signature, narrowing conversion, enum misuse | `MISRA-S4` | `High` | API/port boundary와 직접 연결 |
| Defensive Handling | ignored return, weak null-guard, incomplete error propagation | `MISRA-S5` | `Medium` | safety path면 `High` 승격 가능 |
| Style / Maintainability | scope reduction, redundancy, readability-only pattern | `MISRA-S6` | `Low` | style-only인 경우에만 유지 |

## Required Rule Record Fields

vendor rule ID가 실제로 들어오면 최소 아래 필드를 유지한다.

- `Tool Source`
- `Vendor Rule ID`
- `Vendor Rule Family`
- `Subset ID`
- `Severity`
- `Finding Description`
- `File`
- `Location`
- `Deviation ID` if applicable
- `Review ID`

## Deviation Log Extension Draft

현재 deviation log는 `Rule ID`, `Subset ID`, `Severity`를 포함한다.

vendor tool 도입 시에는 `Rule ID` 필드에 아래 형식을 허용한다.

- `clang:<warning-name>`
- `cppcheck:<finding-type>`
- `vendor:<rule-id>`

예시:

- `vendor:VND-MEM-001`
- `vendor:VND-TYPE-014`

## Report Integration Draft

static analysis report에는 future vendor tool 도입 시 아래 section을 추가한다.

- `Vendor Rule Bucket Summary`
  - family별 count
  - subset별 count
  - severity별 count
- `Open Vendor Deviations`
- `Vendor Rule IDs Requiring Formal Approval`

## Current Decision

1. vendor tool 확정 전까지는 rule family level mapping까지만 유지한다.
2. subset/severity 결정은 현재 baseline 문서와 동일한 구조를 따른다.
3. vendor rule ID는 `tool_specific_misra_mapping.md`의 future MISRA analyzer placeholder를 대체하는 방식으로 들어간다.

## Open Items

| OI ID | Description | Exit Condition |
| --- | --- | --- |
| OI-VM-001 | commercial tool vendor 확정 | tool name과 rule catalog 확보 |
| OI-VM-002 | rule family to exact vendor rule ID mapping | vendor rule matrix 초안 작성 |
| OI-VM-003 | CI/report integration policy | summary/report에 vendor bucket 반영 |

## Follow-up Actions

1. vendor tool 선택 시 rule catalog 확보
2. rule family -> exact rule ID matrix 초안 작성
3. deviation log example entry 추가
