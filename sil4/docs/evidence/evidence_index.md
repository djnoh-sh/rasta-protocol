# Safety Evidence Index

## Document Control

- Document ID: `EVID-000`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 `sil4/docs/evidence` 아래의 정적분석 및 품질 증빙 산출물을 한 곳에서 추적하기 위한 인덱스다.

## Evidence Inventory

| Evidence ID | Artifact | Status | Notes |
| --- | --- | --- | --- |
| EVID-SA-001 | `static_analysis_plan.md` | Draft | 정적분석/MISRA 실행 정책 |
| EVID-SA-002 | `static_analysis_checklist.md` | Draft | 실행 체크리스트 |
| EVID-SA-003 | `misra_deviation_log.md` | Draft | 편차 관리 로그 |
| EVID-SA-004 | `templates/static_analysis_report_template.md` | Draft | 결과 보고서 템플릿 |
| EVID-SA-005 | `reports/static_analysis_report_2026-03-17_sa1.md` | Draft | 첫 baseline 실행 결과 |
| EVID-SA-006 | `../reviews/RV-001_static_analysis_baseline_review.md` | Draft | 첫 evidence review 기록 |
| EVID-SA-007 | `tooling/static_analysis_toolchain_baseline.md` | Draft | baseline tool/version/profile 결정 |
| EVID-SA-008 | `reports/static_analysis_report_2026-03-17_sa2_cppcheck.md` | Draft | 첫 cppcheck baseline 결과 |
| EVID-SA-009 | `../reviews/RV-002_cppcheck_baseline_review.md` | Draft | cppcheck baseline review 기록 |
| EVID-SA-010 | `reports/static_analysis_report_2026-03-17_sa3_followup.md` | Draft | targeted cleanup follow-up 결과 |
| EVID-SA-011 | `../reviews/RV-003_cppcheck_followup_review.md` | Draft | follow-up review 기록 |
| EVID-SA-012 | `reports/static_analysis_report_2026-03-17_sa4_residual_cleanup.md` | Draft | residual cleanup 완료 결과 |
| EVID-SA-013 | `../reviews/RV-004_cppcheck_residual_cleanup_review.md` | Draft | residual cleanup review 기록 |
| EVID-SA-014 | `reports/static_analysis_report_2026-03-17_sa5_wider_cleanup.md` | Draft | wider baseline style finding 정리 결과 |
| EVID-SA-015 | `../reviews/RV-005_cppcheck_wider_cleanup_review.md` | Draft | wider cleanup review 기록 |

## Next Expected Additions

1. rule severity mapping 확정
2. CI execution linkage
3. second-tool baseline 검토
