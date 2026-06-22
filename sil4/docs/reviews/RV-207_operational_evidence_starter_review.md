# RV-207 Operational Evidence Starter Review

## Scope
- add a one-shot starter helper that runs the readiness helper chain and the artifact-dir packet runner from a single artifact dir entry

## Inputs
- `sil4/tools/run_operational_evidence_from_artifact.sh`
- `sil4/tools/test_operational_artifact_runner.sh`
- `sil4/docs/evidence/first_operational_evidence_execution_packet.md`
- `sil4/docs/evidence/first_operational_evidence_handoff_sheet.md`

## Findings
- current `R-005` helper chain already covers readiness summary, update note, ready commands, tracker rows, and audit note
- the remaining entry friction was invoking those helpers separately before the actual artifact-dir runner
- a single entry helper is justified because the residual is artifact availability, not helper orchestration

## Decision
- add `run_operational_evidence_from_artifact.sh`
- make artifact runner smoke verify that the starter emits readiness artifacts and then runs the packet path
- include the starter in execution packet and handoff docs

## Result
- once an actual artifact dir exists, execution can start from a single top-level command instead of a manual multi-step helper sequence
