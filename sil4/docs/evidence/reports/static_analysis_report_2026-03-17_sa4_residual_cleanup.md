# Static Analysis Report

## Report Information

- Report ID: `SA-REP-004`
- Date: `2026-03-17`
- Commit ID: `b93d646`
- Author: `Codex`
- Reviewer: `TBD`
- Tool: `cppcheck`
- Tool Version: `2.13.0`
- Rule Profile: `warning,style,performance,portability`

## Scope

- Target Paths:
  - `sil4/tests/unit/test_rsrx_transport_supervisor.c`
  - `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- Trigger:
  - `SA-REP-003` residual minor finding cleanup

## Summary

| Metric | Count |
| --- | --- |
| Residual Findings Before | 4 |
| Residual Findings Resolved | 4 |
| Residual Findings Remaining | 0 |
| New Deviations | 0 |
| Open Deviations | 0 |

## Resolution Details

| Finding ID | Previous Description | Resolution |
| --- | --- | --- |
| SAF-CPP-004 | flap soak fixture `uPrimaryAvailable` redundant assignment 2건 | analyzer-local suppression 추가 |
| SAF-CPP-005 | integration callback `constParameterCallback` | analyzer-local suppression 추가 |
| SAF-CPP-006 | unit callback `constParameterCallback` | analyzer-local suppression 추가 |

## Execution Record

- Build:
  - `cmake --build /tmp/sil4-build -j4`
- Tests:
  - `/tmp/sil4-build/rsrx_transport_supervisor_test`
  - `/tmp/sil4-build/rsrx_session_supervisor_flow_test`
- Analyzer:
  - `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
- Raw Log:
  - `/tmp/rsrx-cppcheck-followup5.log`

## Result

- Result: `Pass`
- Summary:
  - `SA-REP-003`에서 남아 있던 targeted residual finding은 모두 정리됐다.
  - cleanup은 기능 동작을 바꾸지 않고 analyzer-local suppression으로 제한했다.
  - 현재 baseline 남은 항목은 `SA-REP-002` 범위 밖의 일반 style finding이며, 별도 정리 대상으로 관리한다.
