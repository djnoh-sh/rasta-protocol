# RV-108 Redundancy Holdoff Threshold Six Review

## Scope

- `TC-CHM-020`
- `TC-CHM-021`
- `TC-INT-132`
- `TC-INT-133`
- threshold closeout wrapper refresh

## Decision

- current active-standby preferred-recovery policy keeps higher-threshold parity at `6`
- flap-reset semantics still reset accumulated hold count before renewed recovery
- representative closeout coverage now tracks threshold family through `3/4/5/6`

## Notes

- no new low-level policy was introduced beyond extending existing threshold parity
- residual redundancy work remains in broader threshold generalization, switch audit growth, and longer-run stability expansion
