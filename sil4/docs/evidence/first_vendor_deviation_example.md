# First Vendor Deviation Example

## Document Control

- Document ID: `EVID-025`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 future commercial MISRA analyzer 도입 시 deviation이 실제로 어떻게 기록되고 review와 연결되는지 보여주는 첫 예시를 제공한다.

## Context

- 이 예시는 illustrative sample이다.
- 실제 vendor tool과 실제 finding을 반영한 기록은 아니다.
- 목적은 deviation log, vendor matrix sample, report template 사이 연결을 검증하는 것이다.

## Example Finding Summary

| Field | Value |
| --- | --- |
| Tool Source | `vendor-misra-tool` |
| Vendor Rule ID | `VND-DEF-021` |
| Vendor Rule Family | `Defensive Handling` |
| Subset ID | `MISRA-S5` |
| Severity | `Medium` |
| File | `sil4/src/rsrx_platform_adapters.c` |
| Location | `line TBD` |
| Finding Theme | unchecked return on platform port |
| Initial Decision | deviation review required |

## Example Deviation Record

| Deviation ID | Rule ID | Subset ID | Severity | File | Location | Justification | Risk Assessment | Mitigation | Review ID | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| DEV-EX-001 | `vendor:VND-DEF-021` | `MISRA-S5` | `Medium` | `sil4/src/rsrx_platform_adapters.c` | `line TBD` | wrapper path에서 explicit defensive handling이 이미 상위 호출자에 의해 보장된다는 해석 | data safety path 직접 영향 없음, diagnostics path에 한정 | next code touch 시 explicit comment 추가, vendor suppression rationale review | `RV-EX-001` | `Open` |

## Example Review Record Linkage

review는 최소 아래를 확인해야 한다.

1. rule classification이 `MISRA-S5 / Medium`으로 타당한지
2. 실제 safety path에 direct impact가 없는지
3. mitigation이 concrete한지
4. later cleanup 또는 suppression rationale이 추적 가능한지

## Example Report Usage

static analysis report에는 아래처럼 연결한다.

- `High-Risk Findings`
  - 해당 없음
- `Classification Rationale`
  - why `MISRA-S5` and not `MISRA-S4`
- `Deviation References`
  - `DEV-EX-001`
- `Open Vendor Deviations`
  - `vendor:VND-DEF-021`

## Current Decision

1. 이 문서는 actual vendor finding 이전의 reference example이다.
2. deviation log format과 report template가 실제로 수용 가능한지 보여주는 목적에 충분하다.
3. 첫 실제 vendor finding onboarding 절차는 `first_real_vendor_onboarding.md`를 따른다.
4. 첫 실제 vendor finding이 생기면 본 예시는 archived example로 전환하고 실제 record로 대체한다.

## Follow-up Actions

1. 첫 실제 vendor finding 발생 시 본 예시와 비교 검토
2. 실제 review record format에 concrete reviewer/approval chain 추가
3. PR annotation helper와 vendor deviation linkage 검토
