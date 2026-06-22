# Review Record - RV-490 Preferred Recovery Flap Reset Threshold Twenty

## Scope

- `TC-CHM-062`
- `R-003 Redundancy`
- Preferred recovery higher-threshold flap-reset parity

## Findings

- `TC-CHM-062` adds direct channel-manager flap-reset threshold `20` parity.
- The test verifies that the first stable hold is reset by a preferred-channel flap.
- After the preferred channel is restored again, the renewed holdoff requires nineteen held selections and recovers only on the twentieth stable selection.
- The threshold closeout wrapper now includes pure holdoff `3..20` and flap-reset `3..20`.

## Residual

- This closes the current configured threshold target range through `20`; future redundancy work should focus on new redundancy modes, longer-run target evidence, or new selected policy semantics rather than numeric threshold growth.

## Conclusion

Accepted as preferred recovery flap-reset threshold `20` parity evidence.
