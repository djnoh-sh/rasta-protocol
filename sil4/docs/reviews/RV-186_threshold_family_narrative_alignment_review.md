# RV-186 Threshold Family Narrative Alignment Review

## Scope
- align the broader `2..16` threshold-family wording with the current direct threshold-aware terminal outcome coverage `3..16`

## Inputs
- `sil4/docs/roadmap_status.md`
- `RV-184_switch_audit_threshold_sixteen_terminal_outcome_review.md`
- existing holdoff-threshold and flap-reset representative reviews through `16`

## Findings
- broader holdoff-threshold family coverage already spans `2..16` through matrix/integration and flap-reset representative paths
- direct threshold-aware terminal outcome representative coverage now spans `3..16`
- the remaining wording risk is not missing coverage, but conflating the broader `2..16` family narrative with the narrower direct terminal-outcome subfamily

## Decision
- keep `R-003` explicit that:
  - broader threshold-family narrative already includes existing `2..16` holdoff and flap-reset representative coverage
  - direct threshold-aware terminal outcome coverage is currently `3..16`
  - the residual is future family generalization beyond that current split, not a hidden gap inside the present documented family

## Result
- roadmap wording now distinguishes the broader `2..16` family narrative from the direct `3..16` terminal-outcome subfamily more explicitly
