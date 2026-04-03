# RV-204 Operational Evidence Ready Command Review

## Scope
- add a helper that turns readiness preflight output into immediate artifact-dir runner commands for baseline and vendor tracks

## Inputs
- `sil4/tools/render_operational_evidence_ready_commands.sh`
- `sil4/tools/test_operational_evidence_helpers.sh`
- `sil4/docs/evidence/first_operational_evidence_execution_packet.md`
- `sil4/docs/evidence/first_operational_evidence_handoff_sheet.md`

## Findings
- readiness update text closes the bookkeeping gap, but the executor still had to manually reconstruct the next artifact-dir runner command
- that command is deterministic from readiness `summary.env`
- current `R-005` residual should stay at artifact availability, not command assembly

## Decision
- add `render_operational_evidence_ready_commands.sh`
- make operational evidence smoke verify a baseline/vendor command block is rendered when both artifacts are available
- include the helper in packet and handoff docs as the bridge from preflight to actual runner execution

## Result
- readiness preflight can now flow into both update text and executable artifact-dir command text without extra manual assembly
