# RV-196 P3 Residual Compression Review

## Scope
- compress the `P3` roadmap wording so it summarizes the current sequencing closeout state and keeps only actual next parity-growth residuals

## Inputs
- `RV-037_protocol_runtime_residual_scope_review.md`
- `RV-193_protocol_runtime_residual_compression_review.md`
- `sil4/docs/roadmap_status.md`

## Findings
- the `P3` row still carried a long inventory of already-closed sequencing family coverage
- this no longer matched the compressed residual style now used by `R-001`, `P4`, and `P5`
- the present `P3` state is better described as:
  - current sequencing representative family already closed out
  - remaining work shifted to richer sequencing parity growth beyond the current family

## Decision
- rewrite `P3` so it summarizes:
  - current protocol sequencing closeout family
  - actual next sequencing policy-growth residuals
- stop re-listing already-closed sequencing inventory in the phase row

## Result
- roadmap `P3` wording now matches the compressed residual style used across the rest of the roadmap summary and risk sections
