# RV-134 Redundancy Scope After Refresh Origin Review

## Scope
- `R-003` residual scope reassessment after refresh-origin-aware switch audit coverage

## Observation
- current switch audit family is now covered through:
  - `kind/from/to`
  - `reason`
  - trigger channel/event
  - cumulative switch counts
  - immediate-vs-after-holdoff preferred recovery split and cumulative counts
  - preferred-vs-non-preferred no-op origin counts
  - preferred-vs-non-preferred refresh-total origin counts
  - holdoff progress/target/remaining
  - holdoff reset/cycle/completed/aborted/last-state telemetry
  - representative envelope wrappers at unit/integration level

## Judgment
1. current switch audit family no longer has a meaningful observability gap inside the present active-standby holdoff model.
2. roadmap should not present current refresh-origin-aware switch audit parity as an open implementation hole when the remaining work is policy growth beyond the present family.

## Decision
- keep `R-003` focused on:
  - threshold family generalization beyond current `2..16`
  - future switch audit policy growth beyond current taxonomy/counts/reasons/triggers/no-op-origin/refresh-origin/holdoff-progress/remaining/reset/cycle/completion/abort/last-state/immediate-vs-after-holdoff-envelope
  - broader longer-run stability envelope
  - future redundancy mode growth

## Result
- `R-003` is now described as a next-policy-growth residual rather than a missing current-family refresh-origin-aware switch-audit parity gap.
