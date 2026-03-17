# Static Analysis Report

## Report Information

- Report ID: `SA-REP-003`
- Date: `2026-03-17`
- Commit ID: `62126ba`
- Author: `Codex`
- Reviewer: `TBD`
- Tool: `cppcheck`
- Tool Version: `2.13.0`
- Rule Profile: `warning,style,performance,portability`

## Scope

- Target Paths:
  - `sil4/src/rsrx_channel_manager.c`
  - `sil4/tests/unit/test_rsrx_transport_supervisor.c`
  - `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- Trigger:
  - `SA-REP-002`에서 보고된 baseline finding 후속 정리

## Summary

| Metric | Count |
| --- | --- |
| Targeted Findings Before | 5 |
| Targeted Findings Resolved | 3 |
| Targeted Findings Remaining | 4 |
| New Deviations | 0 |
| Open Deviations | 0 |

## Resolved Findings

| Finding ID | Previous Description | Resolution |
| --- | --- | --- |
| SAF-CPP-001 | integration fixture `uSecondaryAvailable` redundant assignment | 중복 대입 제거 |
| SAF-CPP-002 | integration fixture `uReceiveScriptCount` redundant assignment 일부 | scripted receive count 재설정 위치 정리 |
| SAF-CPP-003 | `rsrx_channel_manager.c` redundant assignment | 중복 state assignment 제거 |

## Remaining Findings

| Finding ID | Severity | File | Rule | Description | Planned Action |
| --- | --- | --- | --- | --- | --- |
| SAF-CPP-004 | Minor | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `redundantAssignment` | flap soak 시나리오의 `uPrimaryAvailable` 중복 대입 2건 | fixture helper 도입 또는 시나리오 표현 정리 |
| SAF-CPP-005 | Minor | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `constParameterCallback` | callback signature상 `pvContext` const suggestion | callback typedef 영향 검토 후 일괄 정리 |
| SAF-CPP-006 | Minor | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `constParameterCallback` | callback signature상 `pvContext` const suggestion | callback typedef 영향 검토 후 일괄 정리 |

## Execution Record

- Build:
  - `cmake --build /tmp/sil4-build -j4`
- Tests:
  - `/tmp/sil4-build/rsrx_transport_supervisor_test`
  - `/tmp/sil4-build/rsrx_session_supervisor_flow_test`
- Analyzer:
  - `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
- Raw Log:
  - `/tmp/rsrx-cppcheck-followup4.log`

## Result

- Result: `Pass with Actions`
- Summary:
  - targeted cleanup은 부분적으로 성공했다.
  - 기능 테스트는 유지됐고, baseline에서 직접 지적된 일부 중복 대입은 제거됐다.
  - callback typedef에 연동된 const warning과 flap soak fixture 표현은 후속 정리가 필요하다.
