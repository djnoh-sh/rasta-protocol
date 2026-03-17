# MISRA Deviation Log

## Document Control

- Document ID: `EVID-003`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Usage Rules

- 편차는 규칙 위반이 정당화되는 경우에만 등록한다.
- “관찰 중” 상태는 허용하지 않는다. `Open`, `Approved`, `Closed` 중 하나여야 한다.
- 편차는 코드 변경 전에 생성할 수 없고, 실제 위반 지점이 특정돼야 한다.
- 각 편차는 review record와 연결되어야 한다.

## Deviation Entries

현재 등록된 active deviation 없음.

참고:
- `SA-REP-002`의 finding은 모두 minor cleanup 항목으로 분류됐고, 현재 단계에서는 deviation으로 승격하지 않았다.
- `SA-REP-005`까지의 cppcheck baseline cleanup 결과, active deviation으로 승격할 항목은 없다.

| Deviation ID | Rule ID | Severity | File | Location | Justification | Risk Assessment | Mitigation | Review ID | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 없음 | 없음 | 없음 | 없음 | 없음 | baseline execution에서 신규 편차 미관찰 | residual risk는 toolchain 미확정에 한정 | `RV-001` 후속 action으로 toolchain 확정 | `RV-001` | Closed |

## Status Definitions

- `Open`: 식별되었으나 승인되지 않음
- `Approved`: 승인되었고 mitigation이 정의됨
- `Closed`: 코드 변경 또는 규칙 해석 정리로 더 이상 편차가 아님
