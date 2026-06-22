# RV-368 Redundant Channel Topology Reject Review

## Scope

- `R-003` current channel-manager topology hardening
- `TC-CHM-058` unsupported `REDUNDANT` channel id startup rejection

## Review Questions

1. Does the current channel-manager topology accept only the primary/secondary members that the active-standby policy actually implements?
2. Is `RSRX_TRANSPORT_CHANNEL_REDUNDANT` kept as a future routing-mode placeholder rather than silently admitted into current selection logic?
3. Are test, LLD, traceability, and roadmap wording aligned with that boundary?

## Findings

1. `rsrx_channel_manager_init` now rejects channel descriptors whose id is not `PRIMARY` or `SECONDARY`.
2. `TC-CHM-058` covers both active-standby and single-mode configs containing `RSRX_TRANSPORT_CHANNEL_REDUNDANT`.
3. The channel-manager LLD and test spec now state that `REDUNDANT` is not a current topology member.
4. Traceability and roadmap now link this guard to the current `R-003` topology-hardening baseline.

## Conclusion

- Pass. Future redundancy routing remains explicit future work and is no longer accidentally accepted by the current primary/secondary channel-manager policy.

## Residual

- Future redundancy mode growth still needs a separate policy definition before `RSRX_TRANSPORT_CHANNEL_REDUNDANT` can become an accepted topology member.
