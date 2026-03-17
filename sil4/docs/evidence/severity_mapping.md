# Severity Mapping Baseline

## Document Control

- Document ID: `EVID-017`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 `sil4/` 재구현 코드베이스에서 compiler warning, static analysis finding, synthetic diagnostic, review finding을 어떤 심각도로 분류할지 기준을 고정한다.

## Scope

- 대상:
  - compiler warning gate
  - `cppcheck` baseline finding
  - session/orchestrator diagnostic code
  - review record finding
- 제외:
  - 외부 프로토타입 코드
  - third-party tool 자체 메시지

## Severity Levels

| Level | Meaning | Default Gate Policy |
| --- | --- | --- |
| `Critical` | safety function 위반, fail-safe 상실, protocol integrity 상실 가능 | merge 불가, 즉시 수정 또는 승인된 편차 필요 |
| `High` | 잘못된 상태 전이, timeout/retransmission/redundancy policy 오류, system-level interface 오류 | merge 불가, 수정 또는 formal review 필요 |
| `Medium` | 기능은 유지되나 진단/관찰성/traceability/defensive handling 약화 | 원칙상 수정, 필요 시 review action으로 추적 |
| `Low` | style, readability, non-safety maintainability 이슈 | 누적 관리 가능, 반복 시 정리 |
| `Info` | 참고성 정보, 상태 변화 기록 | 보고만 수행 |

## Mapping Rules

### Compiler Warnings

| Source | Severity | Rule |
| --- | --- | --- |
| `-Werror`에 걸리는 모든 compiler warning | `High` minimum | baseline gate에서 즉시 실패 |
| memory/UB/signature mismatch로 이어질 수 있는 warning | `Critical` | 원인 제거 전 merge 금지 |

### `cppcheck` Findings

| `cppcheck` Type | Default Severity | Adjustment Rule |
| --- | --- | --- |
| null dereference, use-after-free, out-of-bounds 가능성 | `Critical` | product code면 즉시 차단 |
| uninitialized variable, invalid pointer, resource leak | `High` | control-flow 영향 시 `Critical`까지 승격 가능 |
| redundant assignment, variable scope, unread variable | `Low` | safety path 오해 유발 시 `Medium` |
| portability/style warning | `Low` | interface contract 혼동 시 `Medium` |

### Diagnostic Code To Severity

| Diagnostic Code | Severity |
| --- | --- |
| `RSRX_DIAG_NONE` | `Info` |
| `RSRX_DIAG_INFO_STATE_TRANSITION` | `Info` |
| `RSRX_DIAG_INFO_OPERATIONAL_EVENT` | `Info` |
| `RSRX_DIAG_WARN_REJECTED_EVENT` | `Medium` |
| `RSRX_DIAG_WARN_IGNORED_EVENT` | `Medium` |
| `RSRX_DIAG_ERROR_TIMEOUT` | `High` |
| `RSRX_DIAG_ERROR_PROTOCOL` | `High` |
| `RSRX_DIAG_ERROR_CONFIGURATION` | `High` |
| `RSRX_DIAG_ERROR_INTERFACE` | `High` |
| `RSRX_DIAG_ERROR_INTERNAL_STATE` | `Critical` |

### Review Findings

| Review Finding Type | Default Severity |
| --- | --- |
| safety mechanism 누락 | `High` |
| traceability/evidence gap | `Medium` |
| naming/style/doc drift | `Low` |

## Gate Interpretation

| Gate | Block Condition |
| --- | --- |
| Local development gate | 신규 `High` 이상 finding 존재 |
| CI baseline gate | configure/build/test/cppcheck non-zero exit |
| Safety review gate | `Critical` 미해결 또는 `High` 미승인 finding 존재 |

## Current Baseline Decisions

1. compiler warning은 severity mapping 없이도 `High` minimum으로 즉시 차단한다.
2. 현재 `cppcheck` baseline은 `warning,style,performance,portability`를 사용하며, style 계열은 기본 `Low`로 둔다.
3. synthetic outbound queue overflow reject는 `WARN_REJECTED_EVENT`이므로 현재 baseline에서 `Medium`으로 본다.
4. busy reject threshold escalation이 `ERROR_INTERFACE`로 승격되면 `High`로 본다.

## Residual Limits

- 이 문서는 MISRA rule별 severity까지는 아직 분해하지 않는다.
- second-tool analyzer 도입 시 mapping 재정의가 필요할 수 있다.
- CI workflow는 아직 severity별 artifact summary를 출력하지 않는다.

## Follow-up Actions

1. MISRA subset별 severity 추가
2. CI summary/artifact에 severity bucket 반영
3. review record template에 severity mapping reference 추가
