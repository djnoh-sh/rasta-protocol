# Vendor Rule Matrix Sample

## Document Control

- Document ID: `EVID-024`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 future commercial MISRA analyzer 도입 시 사용할 exact vendor rule matrix의 샘플 엔트리 형식을 제시한다.

목적은 다음과 같다.

1. vendor rule ID level mapping이 실제로 어떤 레코드 구조를 가져야 하는지 구체화한다.
2. deviation log, static analysis report, review record가 어떤 필드를 공유해야 하는지 예시를 제공한다.
3. vendor tool 확정 전에도 문서 체계가 수용 가능한 수준인지 검증한다.

## Scope

- 포함:
  - sample matrix entry format
  - sample deviation linkage
  - sample report linkage
- 제외:
  - 실제 commercial tool vendor 확정
  - 실제 vendor rule number 정확성 보장

## Matrix Fields

| Field | Description |
| --- | --- |
| Tool Source | vendor tool 식별자 |
| Vendor Rule ID | tool-specific rule number |
| Vendor Rule Family | memory / initialization / control-flow / type-interface / defensive / style |
| Subset ID | `MISRA-S1`~`MISRA-S6` |
| Severity | `Critical/High/Medium/Low/Info` |
| Finding Theme | rule이 다루는 핵심 위반 유형 |
| Typical Trigger | 코드에서 이 rule이 관찰될 수 있는 패턴 |
| Default Action | fix / block / deviation review |
| Deviation Eligibility | 편차 허용 여부와 조건 |
| Report Section | report template에서 연결될 section |

## Sample Matrix Entries

| Tool Source | Vendor Rule ID | Vendor Rule Family | Subset ID | Severity | Finding Theme | Typical Trigger | Default Action | Deviation Eligibility | Report Section |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `vendor-misra-tool` | `VND-MEM-001` | Memory | `MISRA-S1` | `Critical` | invalid pointer dereference | null/invalid pointer on transport buffer path | immediate fix, merge block | 원칙상 불가, exceptional safety approval 필요 | `High-Risk Findings`, `Vendor Rule Bucket Summary` |
| `vendor-misra-tool` | `VND-INIT-004` | Initialization | `MISRA-S2` | `High` | uninitialized state variable | state machine timeout/control variable not initialized | fix before merge | 제한적 가능, mitigation 필수 | `High-Risk Findings`, `Classification Rationale` |
| `vendor-misra-tool` | `VND-TYPE-014` | Type / Interface | `MISRA-S4` | `High` | narrowing conversion on protocol field | sequence/confirmation field cast mismatch | fix before merge | 제한적 가능, interface review 필요 | `High-Risk Findings`, `Vendor Rule Bucket Summary` |
| `vendor-misra-tool` | `VND-DEF-021` | Defensive Handling | `MISRA-S5` | `Medium` | unchecked return on platform port | diagnostics/timer/transport status ignored | fix or tracked action | 가능, review action으로 추적 | `Classification Rationale`, `Deviation References` |
| `vendor-misra-tool` | `VND-STY-003` | Style / Maintainability | `MISRA-S6` | `Low` | redundant assignment | local variable overwritten before use | batch cleanup | 가능, cleanup backlog 관리 | `Subset Bucket Summary` |

## Sample Deviation Entry

아래는 future deviation log entry 예시다.

| Deviation ID | Rule ID | Subset ID | Severity | File | Location | Justification | Risk Assessment | Mitigation | Review ID | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| DEV-EX-001 | `vendor:VND-DEF-021` | `MISRA-S5` | `Medium` | `sil4/src/rsrx_platform_adapters.c` | `line TBD` | false positive confirmed on defensive wrapper path | safety path 영향 제한적, diagnostic path only | add explicit code comment and follow-up rule suppression review | `RV-TBD` | `Open` |

## Sample Report Linkage

report template에서 아래와 같이 연결한다.

1. `High-Risk Findings`
   - `VND-MEM-001`, `VND-TYPE-014` 같은 high/critical rule 직접 기록
2. `Vendor Rule Bucket Summary`
   - family별 count
3. `Classification Rationale`
   - 왜 `MISRA-S4/High`로 분류했는지 설명
4. `Deviation References`
   - `vendor:<rule-id>` 형식의 deviation 연결

## Current Decision

1. 이 문서는 exact vendor matrix의 형식 샘플로 유지한다.
2. 실제 vendor 선정 전까지 rule ID와 trigger는 normative가 아니라 illustrative 예시다.
3. vendor tool 확정 후에는 이 샘플을 기반으로 실제 matrix 문서로 승격한다.

## Follow-up Actions

1. selected vendor 기준 exact matrix 초안 작성
2. sample deviation entry를 실제 deviation log 예시와 연결
3. sample report linkage를 첫 vendor baseline report에 반영
4. first actual 운영형 샘플은 `first_actual_vendor_rule_entry_sample.md`를 참조
