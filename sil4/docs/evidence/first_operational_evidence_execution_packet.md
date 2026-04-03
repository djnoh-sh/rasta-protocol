# First Operational Evidence Execution Packet

## Document Control

- Document ID: `EVID-CI-055`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-25`

## Purpose

이 문서는 `R-005`를 실제로 줄이기 위해 필요한 first operational evidence execution packet을 한 곳에 묶는다.

실행자는 이 문서 하나로

1. baseline fetch track
2. first actual vendor finding track

의 준비 상태와 다음 입력 지점을 빠르게 확인할 수 있어야 한다.

## Execution Packet Scope

### Track A: Baseline Fetch Runtime Evidence

- runtime report stub:
  - `reports/baseline_fetch_success_evidence_first_run_stub.md`
- runtime review stub:
  - `reviews/RV-TBD_baseline_fetch_success_runtime_review_stub.md`
- runtime review helper:
  - `../tools/render_baseline_fetch_runtime_review.sh`
- execution runbook:
  - `baseline_fetch_success_execution_runbook.md`
- packet helper:
  - `../tools/render_baseline_fetch_execution_packet.sh`

close target:

- `EVS-001`
- `EVS-002`

### Track B: First Actual Vendor Finding Evidence

- runtime report stub:
  - `reports/first_actual_vendor_finding_set_stub.md`
- runtime review stub:
  - `reviews/RV-TBD_first_actual_vendor_runtime_review_stub.md`
- runtime review helper:
  - `../tools/render_first_actual_vendor_runtime_review.sh`
- actual matrix stub:
  - `vendor_rule_matrix_actual.md`
- matrix helper:
  - `../tools/render_vendor_matrix_actual_row.sh`
- execution runbook:
  - `first_actual_vendor_execution_runbook.md`
- packet helper:
  - `../tools/render_first_actual_vendor_execution_packet.sh`

close target:

- `EVS-003`
- `EVS-004`
- `EVS-005`
- `EVS-006`
- `EVS-007`
- `EVS-008`

## Shared Operational Control

- top-level packet helper:
  - `../tools/render_first_operational_evidence_packet.sh`
- input worksheet:
  - `first_operational_evidence_input_worksheet.md`
- top-level input env helper:
  - `../tools/render_operational_input_env.sh`
- top-level artifact-dir env helper:
  - `../tools/render_operational_input_env_from_artifacts.sh`
- top-level artifact-dir runner:
  - `../tools/run_operational_packet_from_artifacts.sh`
- one-shot actual execution starter:
  - `../tools/run_operational_evidence_from_artifact.sh`
- starter handoff summary helper:
  - `../tools/render_operational_evidence_handoff_summary.sh`
- starter closeout bundle helper:
  - `../tools/render_operational_evidence_closeout_bundle.sh`
- starter patch guidance helper:
  - `../tools/render_operational_evidence_patch_guidance.sh`
- artifact availability preflight helper:
  - `../tools/check_operational_evidence_readiness.sh`
- readiness update helper:
  - `../tools/render_operational_evidence_readiness_update.sh`
- readiness command helper:
  - `../tools/render_operational_evidence_ready_commands.sh`
- readiness tracker-row helper:
  - `../tools/render_operational_evidence_readiness_tracker_rows.sh`
- readiness audit-note helper:
  - `../tools/render_operational_evidence_readiness_audit_note.sh`
- env templates:
  - `templates/first_operational_baseline_input.env`
  - `templates/first_operational_vendor_input.env`
- baseline env helper:
  - `../tools/render_baseline_fetch_input_env.sh`
- baseline ci-log env helper:
  - `../tools/render_baseline_fetch_input_env_from_ci_logs.sh`
- vendor env helper:
  - `../tools/render_vendor_input_env.sh`
- vendor export-metadata env helper:
  - `../tools/render_vendor_input_env_from_export_metadata.sh`
- vendor export-dir env helper:
  - `../tools/render_vendor_input_env_from_export_dir.sh`
- input validator:
  - `../tools/validate_operational_input_env.sh`
- invocation generator:
  - `../tools/render_operational_packet_invocation.sh`
- runner:
  - `../tools/run_operational_packet_from_env.sh`
- packet validator:
  - `../tools/validate_first_operational_packet.sh`
- artifact runner summary validator:
  - `../tools/validate_operational_artifact_runner_summary.sh`
- artifact runner receipt validator:
  - `../tools/validate_operational_artifact_runner_receipt.sh`
- artifact bundle validator:
  - `../tools/validate_operational_artifact_bundle.sh`
- smoke script:
  - `../tools/test_operational_evidence_helpers.sh`
- input pipeline smoke:
  - `../tools/test_operational_input_pipeline.sh`
- artifact runner smoke:
  - `../tools/test_operational_artifact_runner.sh`
- execution tracker:
  - `first_actual_vendor_evidence_set_execution_tracker.md`
- tracker helper:
  - `../tools/render_evidence_execution_tracker_row.sh`
- audit landing zone:
  - `audit_trail_closeout.md`
- audit landing helper:
  - `../tools/render_audit_evidence_update.sh`
- master checklist:
  - `first_actual_vendor_evidence_set_checklist.md`

## Ready State Assessment

| Area | Current State | Note |
| --- | --- | --- |
| Baseline fetch packet | `Prepared` | actual successful PR run만 필요 |
| Vendor finding packet | `Prepared` | actual vendor export만 필요 |
| Tracker linkage | `Prepared` | `EVS-001`~`EVS-008` status row 존재 |
| Audit linkage | `Prepared` | landing zone 정의 완료 |
| Artifact-dir ingress | `Prepared` | baseline log dir / vendor export dir만 있으면 one-shot runner 진입 가능 |
| Artifact-dir auto track detection | `Prepared` | context file만 있으면 track 선택 없이 진입 가능 |
| Artifact-dir auto id generation | `Prepared` | context file 기준 default report/review id 생성 가능 |
| Artifact-dir auto workspace generation | `Prepared` | input env/output dir도 default path로 생성 가능 |
| Artifact-dir runner summary output | `Prepared` | post-run summary env로 follow-up path 바로 확인 가능 |
| Artifact-dir summary validation | `Prepared` | summary env contract를 direct validator로 검증 가능 |
| Artifact-dir runner receipt output | `Prepared` | 사람이 바로 읽는 post-run receipt markdown 생성 가능 |
| Artifact-dir receipt validation | `Prepared` | receipt markdown contract를 direct validator로 검증 가능 |
| Artifact-dir bundle validation | `Prepared` | packet+summary+receipt를 one-shot validator로 검증 가능 |
| Artifact-dir direct runner regression | `Prepared` | baseline/vendor direct smoke 준비 완료 |
| Artifact availability preflight | `Prepared` | baseline/vendor actual artifact availability를 summary/env 하나로 바로 점검 가능 |

## Execution Entry Conditions

packet 실행은 아래 둘 중 하나가 성립할 때 시작한다.

1. first successful `sil4-ci` baseline fetch PR run 확보
2. first actual vendor analyzer export 확보

## Packet Exit Condition

다음이 모두 성립하면 first operational evidence packet은 완료된 것으로 본다.

1. `EVS-001`~`EVS-008`이 모두 `Closed`
2. tracker가 closed reference/date를 포함
3. audit trail이 actual evidence links를 포함
4. roadmap의 `R-005`가 operational maintenance 수준으로 축소

## Invocation Examples

### Baseline Track

```bash
sil4/tools/run_operational_packet_from_env.sh \
  --track baseline \
  --input sil4/docs/evidence/templates/first_operational_baseline_input.env
```

### Vendor Track

```bash
sil4/tools/run_operational_packet_from_env.sh \
  --track vendor \
  --input sil4/docs/evidence/templates/first_operational_vendor_input.env
```

Actual execution:

```bash
sil4/tools/run_operational_packet_from_env.sh \
  --track baseline \
  --input <filled-env-file> \
  --execute
```

## Notes

- 이 문서는 새 증빙 구조를 만들지 않는다.
- 실제 실행 시에는 각 track runbook를 따라가고, 본 문서는 packet-level entry/exit 판단만 담당한다.
- top-level packet helper는 baseline/vendor 어느 track를 실행할지 선택하고 해당 packet helper로 위임한다.
- execute mode runner는 packet generation 뒤 validator까지 수행한다.
- helper chain regression은 `test_operational_evidence_helpers.sh`로 baseline/vendor 둘 다 한 번에 점검한다.
- top-level input collection부터 runner execute까지의 regression은 `test_operational_input_pipeline.sh`로 점검한다.
- top-level artifact-dir one-shot runner regression은 `test_operational_artifact_runner.sh`로 점검한다.
- actual artifact availability preflight는 `check_operational_evidence_readiness.sh`로 baseline/vendor 둘 다 한 번에 점검할 수 있다.
- actual artifact dir 하나로 readiness helper chain과 artifact-dir runner를 연속 실행할 때는 `run_operational_evidence_from_artifact.sh`를 사용한다.
- starter work dir를 handoff용 summary로 다시 묶을 때는 `render_operational_evidence_handoff_summary.sh`를 사용한다.
- starter work dir를 tracker/audit closeout-ready bundle로 다시 묶을 때는 `render_operational_evidence_closeout_bundle.sh`를 사용한다.
- starter work dir를 실제 문서 반영 지점 기준으로 정리할 때는 `render_operational_evidence_patch_guidance.sh`를 사용한다.
- preflight 결과를 tracker/handoff update snippet으로 바꿀 때는 `render_operational_evidence_readiness_update.sh`를 사용한다.
- preflight 결과를 실제 artifact-dir runner command block으로 바꿀 때는 `render_operational_evidence_ready_commands.sh`를 사용한다.
- preflight 결과를 tracker row 초안으로 바꿀 때는 `render_operational_evidence_readiness_tracker_rows.sh`를 사용한다.
- preflight 결과를 audit-trail pre-execution note로 바꿀 때는 `render_operational_evidence_readiness_audit_note.sh`를 사용한다.
- artifact-dir entry는 context file 기준으로 baseline/vendor track를 자동 판별할 수 있다.
- artifact-dir runner는 report/review id를 넘기지 않아도 context file 기준 default id를 생성할 수 있다.
- artifact-dir runner는 input env와 output dir를 넘기지 않아도 default workspace path를 생성할 수 있다.
- artifact-dir runner는 summary env를 남겨 follow-up에 필요한 manifest/input path를 바로 찾게 한다.
- artifact-dir runner는 summary env 생성 뒤 validator까지 수행한다.
- artifact-dir runner는 사람이 바로 볼 receipt markdown도 함께 생성한다.
- artifact-dir runner는 receipt markdown 생성 뒤 validator까지 수행한다.
- artifact-dir runner는 packet+summary+receipt를 bundle validator로 한 번 더 검증한다.
- helper path 기준 `R-005` residual은 더 이상 남지 않았고, 남은 것은 actual artifact availability뿐이다.
