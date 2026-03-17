# Static Analysis Report

## Report Information

- Report ID: `SA-REP-005`
- Date: `2026-03-17`
- Commit ID: `Pending`
- Author: `Codex`
- Reviewer: `TBD`
- Tool: `cppcheck`
- Tool Version: `2.13.0`
- Rule Profile: `warning,style,performance,portability`

## Scope

- Target Paths:
  - `sil4/src/rsrx_orchestrator.c`
  - `sil4/tests/unit/test_rsrx_api.c`
  - `sil4/tests/unit/test_rsrx_codec_contract.c`
  - `sil4/tests/unit/test_rsrx_config_validator.c`
  - `sil4/tests/unit/test_rsrx_platform_adapters.c`
  - `sil4/tests/unit/test_rsrx_platform_contract.c`
  - `sil4/tests/unit/test_rsrx_transport_contract.c`
- Trigger:
  - wider baseline style finding triage after `SA-REP-004`

## Summary

| Metric | Count |
| --- | --- |
| Wider Baseline Findings Before | 35 |
| Wider Baseline Findings Resolved | 35 |
| Wider Baseline Findings Remaining | 0 |
| New Deviations | 0 |
| Open Deviations | 0 |

## Resolution Details

| Finding ID | Previous Description | Resolution |
| --- | --- | --- |
| SAF-CPP-007 | orchestrator `variableScope` 1건 | local variable scope reduction |
| SAF-CPP-008 | callback-local `constVariablePointer` 2건 | local const pointer cleanup |
| SAF-CPP-009 | contract tests `unreadVariable` 다수 | explicit field assertions 추가 |
| SAF-CPP-010 | callback signature `constParameterCallback` 2건 | callback signature 유지, analyzer-local suppression |
| SAF-CPP-011 | contract tests `knownConditionTrueFalse` 5건 | contract assertion 유지, analyzer-local suppression |
| SAF-CPP-012 | config validator fixture `unusedStructMember` 1건 | opaque fixture context 단순화 |

## Execution Record

- Build:
  - `cmake --build /tmp/sil4-build -j4`
- Tests:
  - `/tmp/sil4-build/rsrx_api_test`
  - `/tmp/sil4-build/rsrx_platform_adapters_test`
  - `/tmp/sil4-build/rsrx_transport_supervisor_test`
  - `/tmp/sil4-build/rsrx_session_supervisor_flow_test`
- Analyzer:
  - `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
- Raw Log:
  - `/tmp/rsrx-cppcheck-followup8.log`

## Result

- Result: `Pass`
- Summary:
  - `SA-REP-002` 이후 누적 관리하던 wider baseline style finding이 현재 baseline 범위에서 0건이 되었다.
  - suppressions는 callback signature compatibility와 contract-layout assertion에 한정했다.
  - 신규 deviation은 생성하지 않았다.
