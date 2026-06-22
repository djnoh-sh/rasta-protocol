# RV-136 Switch Audit Bypass Reason Split Review

## Scope
- current switch audit policy growth with explicit active-loss bypass preferred-recovery reason split

## Observation
- current preferred recovery audit already distinguished:
  - immediate preferred recovery
  - preferred recovery after holdoff progress
- but active-loss bypass recovery was still folded into the generic after-holdoff bucket even though it is triggered by a different event path and policy exception.

## Judgment
1. active-loss bypass is not just another ordinary holdoff-complete recovery; it is a separate policy path where secondary loss overrides remaining holdoff.
2. last-event reason and cumulative count should make that distinction explicit.

## Decision
- keep broad `uHoldoffPreferredRecoverySwitchCount` for holdoff-progress-based recoveries
- add explicit bypass-specific reason and cumulative count:
  - `RSRX_SUPERVISOR_SWITCH_REASON_PREFERRED_RECOVERY_BYPASS_ACTIVE_LOSS`
  - `uBypassPreferredRecoverySwitchCount`
- validate with existing holdoff-progress path and existing active-loss bypass representative path

## Result
- current switch audit family now distinguishes immediate recovery, ordinary after-holdoff recovery, and bypass recovery as separate preferred-recovery audit reasons.
