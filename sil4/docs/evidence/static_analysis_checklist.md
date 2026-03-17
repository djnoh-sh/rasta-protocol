# Static Analysis Execution Checklist

## Document Control

- Document ID: `EVID-002`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Pre-Run

- 대상 커밋 ID를 기록했다.
- 분석 대상 파일 목록을 확정했다.
- 빌드가 성공한다.
- 관련 Req ID / Design ID / Test ID를 기록했다.
- 기존 open deviation을 확인했다.

## Run

- compiler warning gate를 실행했다.
- 선택된 정적분석 규칙셋으로 대상 파일을 분석했다.
- 결과 raw log를 보관했다.
- 결과 요약 보고서를 작성했다.

## Post-Run

- 신규 경고를 분류했다.
- false positive 여부를 검토했다.
- 신규 편차가 있으면 `misra_deviation_log.md`에 등록했다.
- 리뷰 기록에 분석 결과를 연결했다.
- 추적성 문서가 필요한 경우 갱신했다.

## Sign-Off

- Implementer:
- Reviewer:
- Date:
- Report ID:
- Result: Pass / Pass with Deviation / Rework Required
