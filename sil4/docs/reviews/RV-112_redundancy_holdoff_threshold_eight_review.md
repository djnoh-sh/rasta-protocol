# RV-112 Redundancy Holdoff Threshold Eight Review

## Scope

- `TC-CHM-024` preferred recovery holdoff threshold `8` parity
- `TC-CHM-025` preferred recovery holdoff threshold `8` flap-reset parity
- `TC-INT-137` preferred recovery holdoff threshold `8` integration
- `TC-INT-138` preferred recovery holdoff threshold `8` flap-reset integration

## Decision

- current active-standby holdoff policy preserves threshold parity at `8`
- flap-reset continues to reset accumulated preferred-recovery holdoff even at threshold `8`
- threshold closeout family now covers `3/4/5/6/7/8` with and without flap reset

## Residual View

- current residual is not threshold parity inside the current family
- remaining `R-003` work is broader threshold generalization, switch audit policy growth, and longer-run stability envelope growth
