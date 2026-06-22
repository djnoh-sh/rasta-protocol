# Static Analysis Report SA-REP-006

## Report Information

- Report ID: `SA-REP-006`
- Date: `2026-03-17`
- Commit ID: `6c52185ad1d2a1e4fe527d5c06204a728d3ea596`
- Author: `Codex`
- Reviewer: `Pending RV-012`
- Tool: `clang`
- Tool Version: `18.1.3`
- Rule Profile: `CMAKE_C_COMPILER=clang`, existing project warning gate

## Scope

- Target Paths:
  - `sil4/include`
  - `sil4/src`
  - `sil4/tests/unit`
  - `sil4/tests/integration`
- Excluded Paths:
  - prototype code outside `sil4/`
  - temporary build output
- Trigger:
  - second-tool baseline first-run evidence

## Summary

| Metric | Count |
| --- | --- |
| Files Analyzed | 37 |
| New Findings | 0 |
| Open Findings | 0 |
| New Deviations | 0 |
| Open Deviations | 0 |
| Test Executables Run | 13 |

## Execution Record

1. configure:
   - `cmake -S sil4 -B /tmp/sil4-clang-build -DCMAKE_C_COMPILER=clang`
2. build:
   - `cmake --build /tmp/sil4-clang-build -j4`
3. executable verification:
   - ran 13 unit/integration executables from `/tmp/sil4-clang-build`
4. logs:
   - build: `/tmp/rsrx-clang-build.log`
   - tests: `/tmp/rsrx-clang-tests.log`
   - configure output captured in terminal session during run

## High-Risk Findings

| Finding ID | Severity | File | Rule | Description | Action |
| --- | --- | --- | --- | --- | --- |
| 없음 | 없음 | 없음 | 없음 | high-risk finding 미관찰 | 없음 |

## Tool-Specific Classification Note

- 이번 실행에서 `clang` warning은 발생하지 않았다.
- 따라서 subset classification은 신규 finding에 대해 적용되지 않았다.
- 분류 기준은 `sil4/docs/evidence/tool_specific_misra_mapping.md`를 참조한다.

## Deviation References

- Related Deviation IDs: 없음

## Result

- Result: `Pass`
- Summary:
  - `clang`을 second-tool candidate로 실제 실행 가능한 상태임을 확인했다.
  - 현재 코드베이스는 `clang 18.1.3` 기준 build clean 상태다.
  - `ctest` 등록은 비어 있어, 이번 baseline에서는 executable direct-run 방식으로 검증했다.
