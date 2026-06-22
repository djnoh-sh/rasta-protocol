# RV-211 Operational Evidence Actual Close Guidance Review

## Scope
- add a helper that maps artifact-runner summary output to the concrete tracker/audit artifacts used for actual close work

## Inputs
- `sil4/tools/render_operational_evidence_actual_close_guidance.sh`
- `sil4/tools/test_operational_artifact_runner.sh`
- `sil4/docs/evidence/first_operational_evidence_execution_packet.md`
- `sil4/docs/evidence/first_operational_evidence_handoff_sheet.md`

## Findings
- starter-side helpers cover pre-execution readiness and document update preparation
- after actual packet generation, the remaining manual step was locating the generated tracker row and audit update artifacts for close work
- that mapping is deterministic from `artifact_runner_summary.env`

## Decision
- add `render_operational_evidence_actual_close_guidance.sh`
- make artifact-runner smoke verify that the helper renders actual close guidance for both baseline and vendor outputs
- include the helper in packet and handoff docs

## Result
- actual close work can now start from the artifact-runner summary without manual inspection of generated output directories
