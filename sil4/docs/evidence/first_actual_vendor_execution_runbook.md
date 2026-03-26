# First Actual Vendor Execution Runbook

## Document Control

- Document ID: `EVID-CI-053`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-25`

## Purpose

이 문서는 `EVS-003`~`EVS-008`을 first actual vendor finding export로 채울 때 필요한 정확한 실행/채움 순서를 고정한다.

목표는 first actual vendor analyzer export가 확보됐을 때 추가 해석 없이

1. vendor runtime report를 채우고
2. vendor runtime review를 작성하고
3. actual matrix와 tracking link를 갱신하고
4. tracker와 audit trail을 닫는 것

이다.

## Preconditions

다음이 모두 성립해야 한다.

1. vendor tool name/version/profile이 식별된다.
2. raw finding export 또는 secured attachment reference가 있다.
3. vendor rule id가 식별 가능하다.
4. file/location이 역추적 가능하다.
5. current mapping reference로 subset/severity classification이 가능하다.

## Evidence Sources

actual evidence는 아래 source에서만 채운다.

- raw vendor export or secured attachment reference
- capture or workflow run page
- vendor rule id / file / location metadata
- `vendor_rule_id_mapping_draft.md`
- `misra_subset_severity.md`
- `severity_mapping.md`
- `first_real_vendor_onboarding.md`

## Fill-In Procedure

helper:

- `sil4/tools/render_first_actual_vendor_evidence.sh`
- `sil4/tools/render_first_actual_vendor_execution_packet.sh`
- `sil4/tools/render_first_operational_evidence_packet.sh --track vendor`

### 0. Vendor Input Env Generation

optional helper:

- `sil4/tools/render_vendor_input_env.sh`
- `sil4/tools/render_vendor_input_env_from_export_metadata.sh`

helper output:

- vendor track packet/runner용 env input file

metadata shortcut:

- actual vendor export metadata를 env file 하나로 정리한 뒤 `render_vendor_input_env_from_export_metadata.sh`로 packet env input으로 바로 변환할 수 있다.

### 1. Runtime Report

target artifact:

- `reports/first_actual_vendor_finding_set_stub.md`

fill:

1. `Execution Date`
2. `Tool Source`
3. `Commit ID`
4. `PR or Branch`
5. `Workflow Run ID` or capture run id
6. raw evidence reference fields
7. vendor rule id / family / subset / severity
8. file / location
9. initial decision
10. downstream artifact links

helper input:

1. run metadata
2. raw evidence reference metadata
3. vendor rule classification fields
4. downstream artifact references

helper output:

- current stub structure와 동일한 runtime report markdown
- packet helper를 쓰면 report/review/matrix/tracker/audit snippet이 한 번에 scaffold된다.

### 2. Runtime Review

target artifact:

- `reviews/RV-TBD_first_actual_vendor_runtime_review_stub.md`

optional helper:

- `sil4/tools/render_first_actual_vendor_runtime_review.sh`

verify:

1. raw evidence reference is traceable
2. rule id / subset / severity / file / location are consistent
3. decision and tracking link are consistent
4. actual matrix entry and audit trail update exist
5. evidence index / tracker / roadmap linkage updated

helper input:

1. report reference
2. runtime report id
3. raw evidence reference
4. rule classification fields
5. matrix/tracking/audit/index/roadmap references

helper output:

- current review stub structure와 동일한 runtime review markdown

### 3. Actual Matrix Update

target artifact:

- `vendor_rule_matrix_actual.md`

optional helper:

- `sil4/tools/render_vendor_matrix_actual_row.sh`

update:

1. actual rule id and family
2. subset and severity
3. file and location
4. initial decision
5. report link
6. review link
7. tracking link
8. status

helper input:

1. tool source and run id
2. rule classification fields
3. report/review/tracking references
4. status

helper output:

- `vendor_rule_matrix_actual.md`의 operational row에 바로 붙일 수 있는 markdown row

### 4. Tracking Update

target artifact:

- `misra_deviation_log.md` or equivalent fix tracking record

update:

1. deviation entry if needed
2. otherwise explicit fix-required or false-positive tracking reference

### 5. Tracker Update

target artifact:

- `first_actual_vendor_evidence_set_execution_tracker.md`

optional helper:

- `sil4/tools/render_evidence_execution_tracker_row.sh`

update:

- `EVS-003` -> `Closed`
- `EVS-004` -> `Closed`
- `EVS-005` -> `Closed`
- `EVS-006` -> `Closed`
- `EVS-007` -> `Closed`
- `EVS-008` -> `Closed` after audit trail update

helper input:

1. `EVS-*` item id
2. status
3. target artifact text
4. execution note text

helper output:

- tracker table에 바로 붙일 수 있는 markdown row

### 6. Audit Trail Update

target artifact:

- `audit_trail_closeout.md`

optional helper:

- `sil4/tools/render_audit_evidence_update.sh --track vendor`

add:

1. actual vendor runtime report link
2. actual vendor runtime review link
3. actual vendor matrix entry link
4. deviation or fix tracking link
5. tracker close status reference

helper input:

1. date
2. report/review references
3. matrix/tracking references
4. tracker reference

helper output:

- vendor landing zone에 붙일 markdown update block

### 7. Packet Validation

optional helper:

- `sil4/tools/validate_first_operational_packet.sh --track vendor`
- `sil4/tools/test_operational_evidence_helpers.sh`

helper output:

- generated vendor packet output completeness confirmation

## Expected Completed Set

이 runbook가 완료되면 최소 아래가 닫혀 있어야 한다.

1. `EVS-003`
2. `EVS-004`
3. `EVS-005`
4. `EVS-006`
5. `EVS-007`
6. `EVS-008`

## Notes

- first actual vendor finding은 sample 문서가 아니라 본 runbook의 target artifact를 직접 채운다.
- deviation이 필요 없더라도 tracking link는 비워두지 않는다.
- second actual vendor finding 이후에는 본 runbook를 반복 적용하되, matrix와 tracker는 누적 갱신한다.
- helper는 runtime report 작성 부담을 줄이지만 review/matrix/tracker/audit trail update 자체를 대체하지는 않는다.
- review helper를 사용해도 reviewer는 `Result`, `Summary`를 직접 최종 확정해야 한다.
- matrix helper를 사용해도 실제 matrix 문서 반영과 status 판단은 실행자가 직접 최종 확정해야 한다.
- tracker helper를 사용해도 close status/date/reference의 최종 판단과 문서 반영은 실행자가 직접 확정해야 한다.
- packet helper는 실행용 scaffold를 한 번에 만들지만, 각 artifact의 실제 operational 반영과 close 판단은 여전히 실행자가 직접 확정해야 한다.
