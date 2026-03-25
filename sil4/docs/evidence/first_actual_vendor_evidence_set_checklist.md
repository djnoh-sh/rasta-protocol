# First Actual Vendor Evidence Set Checklist

## Document Control

- Document ID: `EVID-CI-038`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-25`

## Purpose

이 문서는 `R-005`를 실제 운영형 evidence deliverable 단위로 분해하기 위한 체크리스트다.

목표는 다음 두 공백을 ad-hoc가 아니라 명시적 완료 조건으로 관리하는 것이다.

1. first workflow baseline fetch success evidence
2. first actual vendor evidence set

## Evidence Set Definition

first actual vendor evidence set은 최소 아래 산출물을 포함한다.

1. 실제 vendor tool output 또는 export attachment reference 1건
2. vendor finding이 반영된 static analysis report 1건
3. vendor finding review record 1건
4. vendor matrix actual entry 1건 이상
5. misra deviation log 또는 fix-required tracking linkage 1건
6. audit trail closeout 문서에 actual evidence link 반영

## Checklist

| Item ID | Deliverable | Expected Artifact | Completion Rule | Status |
| --- | --- | --- | --- | --- |
| EVS-001 | baseline fetch success runtime evidence | `docs/evidence/reports/baseline_fetch_success_evidence_YYYY-MM-DD.md` | 실제 workflow run id, baseline source, artifact download/extract success, annotation mode, log reference가 채워진 report 1건 | Open |
| EVS-002 | baseline fetch review | `docs/reviews/RV-TBD_baseline_fetch_success_runtime_review.md` | EVS-001의 runtime field와 workflow linkage를 검토한 review 1건 | Open |
| EVS-003 | vendor raw evidence reference | workflow artifact link 또는 secured attachment reference | 첫 actual vendor finding의 raw output/source reference가 report에서 역추적 가능해야 함 | Open |
| EVS-004 | vendor finding report | `docs/evidence/reports/static_analysis_report_YYYY-MM-DD_vendor1.md` | vendor finding이 subset/severity/rule-id 기준으로 분류된 report 1건 | Open |
| EVS-005 | vendor finding review | `docs/reviews/RV-TBD_first_actual_vendor_runtime_review.md` | EVS-004를 근거로 한 review 1건 | Open |
| EVS-006 | vendor matrix actual entry | `vendor_rule_matrix_actual.md` 또는 operational matrix section | sample이 아니라 실제 vendor rule id와 status가 반영된 entry 1건 이상 | Open |
| EVS-007 | deviation or fix tracking link | `misra_deviation_log.md` 또는 equivalent fix tracking record | first actual vendor finding이 deviation인지 fix-required인지가 audit trail에서 추적 가능해야 함 | Open |
| EVS-008 | audit trail update | `audit_trail_closeout.md` | actual baseline fetch evidence와 actual vendor evidence link가 closeout 문서에 반영되어야 함 | Open |

## Entry Criteria

다음 중 하나가 충족되면 checklist execution을 시작한다.

1. `sil4-ci` workflow에서 baseline artifact fetch가 실제 성공한 첫 run이 나온 경우
2. first actual vendor analyzer finding export가 확보된 경우

## Exit Criteria

다음이 모두 충족되면 `R-005`의 첫 operational evidence gap은 축소된 것으로 본다.

1. `EVS-001`, `EVS-002` 완료
2. `EVS-003`~`EVS-008` 완료
3. `roadmap_status.md`의 `R-005`가 template/sample gap이 아니라 operational evidence maintenance 수준으로 축소

## Notes

- 이 문서는 template/sample artifact를 대체하지 않는다.
- `baseline_fetch_success_evidence_template.md`, `first_real_vendor_onboarding.md`, `first_actual_vendor_rule_entry_sample.md`를 operational 실행으로 연결하는 checklist 역할만 맡는다.
