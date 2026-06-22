# RV-202 Operational Evidence Readiness Preflight Review

## Scope
- add a top-level readiness preflight helper so `R-005` actual execution can start from an explicit baseline/vendor artifact availability check

## Inputs
- `sil4/tools/check_operational_evidence_readiness.sh`
- `sil4/tools/test_operational_evidence_helpers.sh`
- `sil4/docs/evidence/first_operational_evidence_execution_packet.md`
- `sil4/docs/evidence/first_operational_evidence_handoff_sheet.md`

## Findings
- current `R-005` residual is artifact availability itself, not helper structure
- that means execution readiness should start with a direct answer to:
  - is a baseline artifact dir actually present
  - is a vendor export dir actually present
- existing packet/runbook/handoff docs had execution helpers, but no single top-level preflight summary for both tracks together

## Decision
- add `check_operational_evidence_readiness.sh` as the top-level availability preflight helper
- make it write `summary.md` and `summary.env`
- allow optional fail-fast with `--require-ready`
- include the helper in operational evidence smoke and execution docs

## Result
- `R-005` execution can now begin from an explicit artifact-availability preflight instead of an implicit manual check
