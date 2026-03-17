# MISRA Subset Severity Baseline

## Document Control

- Document ID: `EVID-018`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 `sil4/` 재구현 코드베이스에서 우선 적용할 MISRA subset과 각 subset의 기본 severity를 정의한다.

## Scope

- 대상:
  - 메모리/포인터 안전
  - 초기화/정의되지 않은 동작
  - 제어 흐름과 상태 처리
  - 인터페이스와 타입 일관성
- 제외:
  - full MISRA catalog 전체 분해
  - tool-vendor specific rule numbering

## Baseline Subset

| Subset ID | Category | Description | Default Severity |
| --- | --- | --- | --- |
| MISRA-S1 | Memory Safety | invalid pointer, out-of-bounds, lifetime misuse | `Critical` |
| MISRA-S2 | Initialization | uninitialized read, undefined default state | `High` |
| MISRA-S3 | Control Flow | unreachable/error path omission, unsafe fall-through, invalid state handling | `High` |
| MISRA-S4 | Interface Integrity | incompatible signature, enum misuse, type narrowing | `High` |
| MISRA-S5 | Defensive Robustness | null guard omission, unchecked return, incomplete error propagation | `Medium` |
| MISRA-S6 | Maintainability Style | redundant assignment, variable scope, readability-only issues | `Low` |

## Mapping Intent

### Critical

- safety function 상실로 직결될 수 있는 violation
- 예:
  - dangling pointer use
  - out-of-bounds access
  - freed/invalid storage reuse
  - corrupted protocol buffer access

### High

- fail-safe, timeout, retransmission, redundancy, API contract를 깨뜨릴 수 있는 violation
- 예:
  - uninitialized control variable
  - invalid enum/state transition
  - unchecked interface return on safety path
  - narrowing/implicit conversion으로 protocol field 손상

### Medium

- 직접 fail-safe 상실은 아니지만 defensive handling과 diagnosability를 약화하는 violation
- 예:
  - partial error propagation
  - null guard inconsistency
  - incomplete diagnostic/report path

### Low

- style/readability 중심이며 safety effect가 간접적인 violation
- 예:
  - redundant assignment
  - variable scope reduction
  - non-safety naming drift

## Gate Use

| Severity | Gate Handling |
| --- | --- |
| `Critical` | merge 금지, deviation 승인 전까지 block |
| `High` | merge 금지, fix 또는 formal approval 필요 |
| `Medium` | 원칙상 fix, 필요 시 action/open finding으로 추적 |
| `Low` | batch cleanup 허용, 반복 시 정리 |

## Current Decision

1. current baseline에서는 `MISRA-S1`~`MISRA-S4`를 safety gate 핵심 subset으로 본다.
2. `MISRA-S5`는 review action으로 추적 가능하지만 누적 방치는 허용하지 않는다.
3. `MISRA-S6`는 static analysis cleanup backlog로 관리 가능하다.

## Relation To Existing Artifacts

- general severity mapping:
  - `sil4/docs/evidence/severity_mapping.md`
- deviation control:
  - `sil4/docs/evidence/misra_deviation_log.md`
- CI linkage:
  - `sil4/docs/evidence/ci_execution_linkage.md`

## Residual Limits

- 아직 tool-specific MISRA rule IDs에 매핑하지 않았다.
- 아직 commercial MISRA analyzer output format에 연결되지 않았다.

## Follow-up Actions

1. tool-specific rule ID mapping 추가
2. deviation log template에 subset ID 필드 연결
3. CI summary에 subset severity summary 추가
