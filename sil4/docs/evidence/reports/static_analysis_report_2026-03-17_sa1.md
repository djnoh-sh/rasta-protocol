# Static Analysis Report

## Report Information

- Report ID: `SA-REP-001`
- Date: `2026-03-17`
- Commit ID: `7bb0fe8`
- Author: `Codex`
- Reviewer: `TBD`
- Tool: `cmake build warning gate + MISRA checklist review`
- Tool Version: `cmake local build environment`
- Rule Profile: `compiler warnings zero-tolerance, SIL4_REIMPLEMENTATION_RULES.md, CODING_RULES.md`

## Scope

- Target Paths:
  - `sil4/include` (`12` files)
  - `sil4/src` (`10` files)
  - `sil4/tests/unit` (`13` files)
  - `sil4/tests/integration` (`2` files)
- Excluded Paths:
  - `sil4/docs`
  - prototype code outside `sil4/`
  - external toolchain headers
  - generated build artifacts under `/tmp/sil4-build`
- Trigger:
  - `M26 static analysis evidence baseline` 후 첫 실행 결과 고정

## Summary

| Metric | Count |
| --- | --- |
| Files Analyzed | 37 |
| New Findings | 0 |
| Open Findings | 0 |
| New Deviations | 0 |
| Open Deviations | 0 |

## Execution Record

- Build Command:
  - `cmake --build /tmp/sil4-build -j4`
- Result:
  - exit code `0`
  - compiler warning output observed `0`
- Raw Log:
  - `/tmp/rsrx-static-analysis-build.log`

## High-Risk Findings

없음.

## MISRA-Oriented Checklist Result

- 변수 초기화 규칙 위반 관찰 없음
- 동적 메모리 사용 관찰 없음
- 재귀 호출 관찰 없음
- `goto` 사용 관찰 없음
- `switch` default 누락 신규 관찰 없음
- unsigned literal suffix 신규 위반 관찰 없음

## Deviation References

- Related Deviation IDs:
  - 없음

## Result

- Result: `Pass`
- Summary:
  - 현재 기준의 최소 정적분석 게이트는 통과했다.
  - 본 보고서는 compiler warning gate와 MISRA-oriented checklist review 기반의 baseline evidence다.
  - 전용 정적분석 도구와 rule profile은 아직 확정되지 않았으므로, 본 보고서는 `SA-1 baseline` 성격으로만 사용한다.
