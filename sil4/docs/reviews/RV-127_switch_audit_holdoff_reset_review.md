# RV-127 Switch Audit Holdoff Reset Review

## Summary
- switch audit telemetry에 holdoff reset 관찰값을 추가했다.
- preferred primary가 holdoff 누적 중 `CHANNEL_DOWN` 되면 no-switch reset reason과 cumulative reset count가 supervisor report에 남는다.

## Added Observability
- `uHoldoffResetCount`
- `eLastSwitchReason = RSRX_SUPERVISOR_SWITCH_REASON_HOLDOFF_RESET_CHANNEL_DOWN`

## Covered Contract
- holdoff no-op 뒤 preferred primary `CHANNEL_DOWN`:
  - progress resets to `0`
  - remaining resets to configured target
  - no switch occurs
  - reset reason/count is recorded
- renewed hold after reset:
  - progress starts again from the first hold step

## Coverage
- `TC-SUP-039` switch audit holdoff reset matrix
- `TC-INT-157` switch audit holdoff reset integration

## Result
- current switch audit family is now covered through:
  - `kind`
  - `from/to`
  - `reason`
  - trigger channel/event
  - cumulative counts
  - holdoff progress/target/remaining
  - holdoff reset reason/count

## Residual
- current active-standby holdoff audit family gap is narrower again.
- remaining work is future audit-policy growth beyond the current trigger-aware, holdoff-progress/remaining/reset-aware envelope.
