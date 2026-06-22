# Static Analysis Report

## Report Information

- Report ID: `SA-REP-002`
- Date: `2026-03-17`
- Commit ID: `bea5587`
- Author: `Codex`
- Reviewer: `TBD`
- Tool: `cppcheck`
- Tool Version: `2.13.0`
- Rule Profile: `warning,style,performance,portability`

## Scope

- Target Paths:
  - `sil4/include`
  - `sil4/src`
  - `sil4/tests/unit`
  - `sil4/tests/integration`
- Excluded Paths:
  - `sil4/docs`
  - prototype code outside `sil4/`
  - generated build artifacts
- Trigger:
  - baseline toolchain 확정 직후 first `cppcheck` execution

## Summary

| Metric | Count |
| --- | --- |
| Files Analyzed | 22 |
| New Findings | 5 |
| Open Findings | 5 |
| New Deviations | 0 |
| Open Deviations | 0 |

## Execution Record

- Command:
  - `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
- Result:
  - exit code `0`
- Raw Log:
  - `/tmp/rsrx-cppcheck.log`

## High-Risk Findings

없음.

## Findings

| Finding ID | Severity | File | Rule | Description | Action |
| --- | --- | --- | --- | --- | --- |
| SAF-CPP-001 | Minor | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `redundantAssignment` | `xTransport.uSecondaryAvailable` 중복 대입 2건 | test fixture assignment 정리 |
| SAF-CPP-002 | Minor | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `redundantAssignment` | `xTransport.uReceiveScriptCount` 중복 대입 2건 | scripted receive setup 정리 |
| SAF-CPP-003 | Minor | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `constVariablePointer` | `test_clock_context_t * pxContext`는 const pointer 가능 | const correctness 정리 |

## Deviation References

- Related Deviation IDs:
  - 없음

## Result

- Result: `Pass with Actions`
- Summary:
  - baseline `cppcheck` scan은 성공적으로 완료되었다.
  - 현재 발견된 항목은 모두 test code style 수준의 minor finding이며, 제품 코드의 safety-critical defect로 분류되지는 않았다.
  - 다음 단계에서 findings를 정리하고, 보고서를 follow-up review와 연결해야 한다.
