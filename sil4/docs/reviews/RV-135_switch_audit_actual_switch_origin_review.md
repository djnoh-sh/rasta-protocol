# RV-135 Switch Audit Actual Switch Origin Review

## Scope
- current switch audit policy growth with cumulative actual-switch trigger-origin observability

## Observation
- current switch audit family already covered:
  - `kind/from/to`
  - `reason`
  - trigger channel/event
  - cumulative switch counts
  - no-op origin counts
  - refresh-total origin counts
  - holdoff progress/reset/cycle outcome/state
- but actual switches themselves were not yet split by preferred-triggered vs non-preferred-triggered cumulative origin.

## Judgment
1. refresh-total origin counts alone are not enough to distinguish actual failover/recovery switches from no-op refresh traffic.
2. active-loss bypass introduces a real non-preferred-triggered switch path that should be observable cumulatively, not only through the last-event trigger field.

## Decision
- add cumulative actual-switch trigger-origin counts for:
  - preferred-triggered actual switches
  - non-preferred-triggered actual switches
- validate with:
  - unit failover -> hold -> active-loss bypass path
  - integration holdoff active-loss bypass representative flow

## Result
- current switch audit family now covers actual-switch trigger-origin parity in addition to no-op origin parity and refresh-total origin parity.
