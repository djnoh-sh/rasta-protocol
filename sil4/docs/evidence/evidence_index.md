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
| EVID-SA-018 | `../reviews/RV-017_static_analysis_report_template_review.md` | Draft | report template refinement review |
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
| EVID-DES-001 | `reports/outbound_queue_policy_evidence_2026-03-17.md` | Draft | bounded outbound queue policy evidence note |
| EVID-DES-002 | `../reviews/RV-006_outbound_queue_policy_review.md` | Draft | outbound queue policy safety review |
| EVID-CI-001 | `ci_execution_linkage.md` | Draft | local verification과 CI workflow linkage 기준 |
| EVID-CI-002 | `../reviews/RV-007_ci_execution_linkage_review.md` | Draft | CI linkage safety review |
| EVID-CI-003 | `severity_mapping.md` | Draft | compiler/static-analysis/diagnostic severity 기준 |
| EVID-CI-004 | `../reviews/RV-008_severity_mapping_review.md` | Draft | severity mapping safety review |
| EVID-CI-005 | `misra_subset_severity.md` | Draft | MISRA subset severity baseline |
| EVID-CI-006 | `../reviews/RV-009_misra_subset_severity_review.md` | Draft | MISRA subset severity safety review |
| EVID-CI-007 | `tool_specific_misra_mapping.md` | Draft | tool-specific MISRA subset mapping baseline |
| EVID-CI-008 | `../reviews/RV-010_tool_specific_misra_mapping_review.md` | Draft | tool-specific MISRA mapping safety review |
| EVID-CI-009 | `second_tool_baseline_candidates.md` | Draft | second-tool baseline candidate decision |
| EVID-CI-010 | `../reviews/RV-011_second_tool_baseline_candidates_review.md` | Draft | second-tool candidate safety review |
| EVID-CI-011 | `../reviews/RV-013_ci_subset_summary_review.md` | Draft | CI subset summary safety review |
| EVID-CI-012 | `vendor_rule_id_mapping_draft.md` | Draft | vendor rule ID mapping draft |
| EVID-CI-013 | `../reviews/RV-014_vendor_rule_id_mapping_review.md` | Draft | vendor rule mapping safety review |
| EVID-CI-014 | `ctest_registration_strategy.md` | Draft | ctest registration strategy baseline |
| EVID-CI-015 | `../reviews/RV-015_ctest_registration_strategy_review.md` | Draft | ctest strategy safety review |
| EVID-CI-016 | `pr_annotation_strategy.md` | Draft | PR annotation policy baseline |
| EVID-CI-017 | `../reviews/RV-016_pr_annotation_strategy_review.md` | Draft | PR annotation strategy safety review |
| EVID-CI-018 | `vendor_rule_matrix_sample.md` | Draft | exact vendor matrix sample entry |
| EVID-CI-019 | `../reviews/RV-018_vendor_rule_matrix_sample_review.md` | Draft | vendor matrix sample safety review |
| EVID-CI-020 | `../reviews/RV-019_pr_annotation_helper_review.md` | Draft | PR annotation helper implementation review |
| EVID-CI-023 | `../reviews/RV-021_pr_comment_api_linkage_review.md` | Draft | PR comment API linkage review |
| EVID-CI-024 | `delta_based_annotation_policy.md` | Draft | delta-based PR annotation policy |
| EVID-CI-025 | `../reviews/RV-022_delta_based_annotation_policy_review.md` | Draft | delta-based annotation policy review |
| EVID-CI-026 | `../reviews/RV-023_delta_aware_helper_review.md` | Draft | delta-aware helper implementation review |
| EVID-CI-027 | `first_real_vendor_onboarding.md` | Draft | first real vendor finding onboarding procedure |
| EVID-CI-028 | `../reviews/RV-024_first_real_vendor_onboarding_review.md` | Draft | first real vendor onboarding procedure review |
| EVID-CI-029 | `baseline_persistence_source.md` | Draft | baseline summary persistence source policy |
| EVID-CI-030 | `../reviews/RV-025_baseline_persistence_source_review.md` | Draft | baseline persistence source review |
| EVID-CI-031 | `../reviews/RV-026_baseline_artifact_fetch_review.md` | Draft | baseline artifact fetch implementation review |
| EVID-CI-032 | `first_actual_vendor_rule_entry_sample.md` | Draft | first actual vendor rule entry sample |
| EVID-CI-033 | `../reviews/RV-027_first_actual_vendor_rule_entry_review.md` | Draft | first actual vendor rule entry sample review |
| EVID-CI-034 | `audit_trail_closeout.md` | Draft | evidence package audit trail closeout baseline |
| EVID-CI-035 | `../reviews/RV-028_audit_trail_closeout_review.md` | Draft | audit trail closeout review |
| EVID-CI-036 | `reports/baseline_fetch_success_evidence_template.md` | Draft | first baseline fetch success evidence template |
| EVID-CI-037 | `../reviews/RV-029_baseline_fetch_success_evidence_template_review.md` | Draft | baseline fetch success evidence template review |
| EVID-CI-038 | `first_actual_vendor_evidence_set_checklist.md` | Draft | first actual vendor evidence set execution checklist |
| EVID-CI-039 | `../reviews/RV-038_vendor_evidence_set_checklist_review.md` | Draft | vendor evidence set checklist review |
| EVID-CI-040 | `reports/baseline_fetch_success_evidence_first_run_stub.md` | Draft | first baseline fetch runtime evidence stub |
| EVID-CI-041 | `../reviews/RV-039_baseline_fetch_first_run_stub_review.md` | Draft | baseline fetch first-run stub review |
| EVID-CI-042 | `reports/first_actual_vendor_finding_set_stub.md` | Draft | first actual vendor finding evidence stub |
| EVID-CI-043 | `../reviews/RV-040_vendor_finding_first_run_stub_review.md` | Draft | vendor finding first-run stub review |
| EVID-CI-044 | `vendor_rule_matrix_actual.md` | Draft | actual vendor rule matrix operational stub |
| EVID-CI-045 | `../reviews/RV-041_vendor_rule_matrix_actual_stub_review.md` | Draft | vendor rule matrix actual stub review |
| EVID-CI-046 | `../reviews/RV-TBD_baseline_fetch_success_runtime_review_stub.md` | Draft | baseline fetch runtime review operational stub |
| EVID-CI-047 | `../reviews/RV-TBD_first_actual_vendor_runtime_review_stub.md` | Draft | first actual vendor runtime review operational stub |
| EVID-CI-048 | `first_actual_vendor_evidence_set_execution_tracker.md` | Draft | first actual vendor evidence set execution tracker |
| EVID-CI-049 | `../reviews/RV-042_evidence_execution_tracker_review.md` | Draft | evidence execution tracker review |
| EVID-CI-050 | `../reviews/RV-043_audit_landing_zone_review.md` | Draft | audit trail operational landing zone review |
| EVID-CI-051 | `baseline_fetch_success_execution_runbook.md` | Draft | baseline fetch success execution runbook |
| EVID-CI-052 | `../reviews/RV-045_baseline_fetch_runbook_review.md` | Draft | baseline fetch runbook review |
| EVID-CI-053 | `first_actual_vendor_execution_runbook.md` | Draft | first actual vendor execution runbook |
| EVID-CI-054 | `../reviews/RV-046_vendor_execution_runbook_review.md` | Draft | vendor execution runbook review |
| EVID-CI-055 | `first_operational_evidence_execution_packet.md` | Draft | first operational evidence execution packet |
| EVID-CI-056 | `../reviews/RV-047_execution_packet_review.md` | Draft | execution packet review |
| EVID-CI-057 | `first_operational_evidence_handoff_sheet.md` | Draft | first operational evidence handoff sheet |
| EVID-CI-058 | `../reviews/RV-048_handoff_sheet_review.md` | Draft | handoff sheet review |
| EVID-CI-059 | `../reviews/RV-049_baseline_fetch_evidence_helper_review.md` | Draft | baseline fetch evidence helper review |
| EVID-CI-060 | `../reviews/RV-050_vendor_evidence_helper_review.md` | Draft | vendor evidence helper review |
| EVID-CI-061 | `../reviews/RV-051_vendor_runtime_review_helper_review.md` | Draft | vendor runtime review helper review |
| EVID-CI-062 | `../reviews/RV-052_vendor_matrix_helper_review.md` | Draft | vendor matrix helper review |
| EVID-CI-063 | `../reviews/RV-053_execution_tracker_helper_review.md` | Draft | execution tracker helper review |
| EVID-CI-064 | `../reviews/RV-054_audit_update_helper_review.md` | Draft | audit update helper review |
| EVID-CI-065 | `../reviews/RV-055_vendor_execution_packet_helper_review.md` | Draft | vendor execution packet helper review |
| EVID-CI-066 | `../reviews/RV-056_baseline_runtime_review_helper_review.md` | Draft | baseline runtime review helper review |
| EVID-CI-067 | `../reviews/RV-057_baseline_execution_packet_helper_review.md` | Draft | baseline execution packet helper review |
| EVID-CI-021 | `first_vendor_deviation_example.md` | Draft | first vendor deviation reference example |
| EVID-CI-022 | `../reviews/RV-020_first_vendor_deviation_example_review.md` | Draft | vendor deviation example safety review |
| EVID-SA-016 | `reports/static_analysis_report_2026-03-17_sa6_clang_baseline.md` | Draft | first clang second-tool baseline result |
| EVID-SA-017 | `../reviews/RV-012_clang_baseline_review.md` | Draft | clang baseline evidence review |

## Next Expected Additions

1. PR annotation workflow helper
2. CI execution linkage
3. first vendor deviation example
4. queue/backpressure policy evidence 추가 확장
5. MISRA subset severity 세분화
6. second-tool rule taxonomy mapping
7. first workflow baseline fetch success evidence
8. first actual vendor evidence set
9. first actual vendor evidence set checklist execution
10. first baseline fetch success runtime report
11. first actual vendor finding runtime report
12. first actual vendor matrix entry
13. first baseline fetch runtime review
14. first actual vendor runtime review
15. first evidence execution tracker status update
16. first baseline fetch runbook execution
17. first actual vendor runbook execution
18. first operational evidence packet execution
19. first operational evidence handoff execution
20. first actual vendor evidence helper execution
21. first actual vendor runtime review helper execution
22. first actual vendor matrix helper execution
23. first evidence execution tracker helper execution
24. first audit trail landing helper execution
25. first vendor execution packet helper execution
26. first baseline runtime review helper execution
27. first baseline execution packet helper execution
