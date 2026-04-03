# RV-210 Operational Evidence Patch Guidance Review

## Scope
- add a helper that maps starter outputs to the concrete document update targets for tracker, handoff, and audit work

## Inputs
- `sil4/tools/render_operational_evidence_patch_guidance.sh`
- `sil4/tools/test_operational_artifact_runner.sh`
- `sil4/docs/evidence/first_operational_evidence_execution_packet.md`
- `sil4/docs/evidence/first_operational_evidence_handoff_sheet.md`

## Findings
- closeout bundle already groups starter outputs by closeout purpose
- the remaining manual step was identifying which repository document each starter artifact should update
- that mapping is deterministic and should be emitted by helper rather than rewritten each run

## Decision
- add `render_operational_evidence_patch_guidance.sh`
- make artifact-runner smoke verify that the helper renders patch guidance from starter output
- include the helper in packet and handoff docs

## Result
- starter output can now be translated into direct repository patch targets without manual file-by-file interpretation
