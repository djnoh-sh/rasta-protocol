# First Operational Evidence Handoff Sheet

## Document Control

- Document ID: `EVID-CI-057`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-25`

## Purpose

이 문서는 first operational evidence execution 직전에 실행자에게 넘길 최소 handoff sheet다.

packet/runbook이 상세 절차를 담는다면, 본 문서는 다음만 빠르게 확인하게 한다.

1. 어떤 track를 지금 실행할 것인가
2. 어떤 artifact가 이미 준비됐는가
3. 무엇이 아직 비어 있는가
4. 실행 후 어디를 갱신해야 하는가

## Current Handoff State

### Track A: Baseline Fetch

| Item | State |
| --- | --- |
| baseline fetch runtime report stub | `Prepared` |
| baseline fetch runtime review stub | `Prepared` |
| baseline fetch runtime review helper | `Prepared` |
| baseline fetch packet helper | `Prepared` |
| baseline fetch execution runbook | `Prepared` |
| actual successful PR run | `Missing` |

### Track B: First Actual Vendor Finding

| Item | State |
| --- | --- |
| vendor runtime report stub | `Prepared` |
| vendor evidence helper | `Prepared` |
| vendor runtime review stub | `Prepared` |
| vendor runtime review helper | `Prepared` |
| actual vendor matrix stub | `Prepared` |
| vendor matrix helper | `Prepared` |
| vendor execution packet helper | `Prepared` |
| vendor execution runbook | `Prepared` |
| actual vendor export | `Missing` |

### Shared Update Targets

| Item | State |
| --- | --- |
| execution tracker | `Prepared` |
| execution tracker helper | `Prepared` |
| audit landing zone | `Prepared` |
| audit landing helper | `Prepared` |
| top-level packet helper | `Prepared` |
| top-level input env helper | `Prepared` |
| top-level artifact-dir env helper | `Prepared` |
| top-level artifact-dir runner | `Prepared` |
| one-shot actual execution starter | `Prepared` |
| artifact-dir auto track detection | `Prepared` |
| artifact-dir auto report/review id generation | `Prepared` |
| artifact-dir auto workspace generation | `Prepared` |
| artifact-dir runner summary output | `Prepared` |
| artifact-dir runner summary validation | `Prepared` |
| artifact-dir runner receipt output | `Prepared` |
| artifact-dir runner receipt validation | `Prepared` |
| artifact-dir bundle validation | `Prepared` |
| artifact availability preflight helper | `Prepared` |
| readiness update helper | `Prepared` |
| readiness command helper | `Prepared` |
| readiness tracker-row helper | `Prepared` |
| readiness audit-note helper | `Prepared` |
| input worksheet | `Prepared` |
| input validator | `Prepared` |
| invocation generator | `Prepared` |
| env templates | `Prepared` |
| env runner | `Prepared` |
| packet validator | `Prepared` |
| helper smoke script | `Prepared` |
| input pipeline smoke script | `Prepared` |
| artifact runner smoke script | `Prepared` |
| direct artifact-dir runner regression | `Prepared` |
| roadmap `R-005` residual wording | `Prepared for artifact-only residual tracking` |

## Immediate Actions

### If Baseline Fetch Run Appears

1. follow `baseline_fetch_success_execution_runbook.md`
2. fill `EVS-001`, `EVS-002`
3. update tracker and audit trail
4. if needed, check readiness first with `sil4/tools/check_operational_evidence_readiness.sh ...`
5. if needed, render readiness note with `sil4/tools/render_operational_evidence_readiness_update.sh ...`
6. if needed, render ready command with `sil4/tools/render_operational_evidence_ready_commands.sh ...`
7. if needed, render tracker row with `sil4/tools/render_operational_evidence_readiness_tracker_rows.sh ...`
8. if needed, render audit note with `sil4/tools/render_operational_evidence_readiness_audit_note.sh ...`
9. if needed, start with `sil4/tools/run_operational_evidence_from_artifact.sh --artifact-dir ...`

### If First Vendor Export Appears

1. follow `first_actual_vendor_execution_runbook.md`
2. fill `EVS-003`~`EVS-008`
3. update tracker, matrix, tracking log, audit trail
4. if needed, check readiness first with `sil4/tools/check_operational_evidence_readiness.sh ...`
5. if needed, render readiness note with `sil4/tools/render_operational_evidence_readiness_update.sh ...`
6. if needed, render ready command with `sil4/tools/render_operational_evidence_ready_commands.sh ...`
7. if needed, render tracker row with `sil4/tools/render_operational_evidence_readiness_tracker_rows.sh ...`
8. if needed, render audit note with `sil4/tools/render_operational_evidence_readiness_audit_note.sh ...`
9. if needed, start with `sil4/tools/run_operational_evidence_from_artifact.sh --artifact-dir ...`

## Done Condition

이 handoff sheet는 아래 둘 중 하나가 생기면 실제 execution으로 전환된다.

1. successful baseline fetch PR run
2. first actual vendor analyzer export

그 전까지는 `Prepared / Missing` 상태를 유지한다.
