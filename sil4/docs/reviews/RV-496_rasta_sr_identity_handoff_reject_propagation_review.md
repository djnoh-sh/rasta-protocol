# Review Record - RV-496 RaSTA SR Identity Handoff Reject Propagation

## Scope

- `TC-CODEC-045`
- `R-006 Codec/security`
- RaSTA SR identity+timestamp handoff mapping

## Findings

- The identity+timestamp handoff bridge now has explicit evidence that timestamp admission causal-order rejection is propagated after identity admission succeeds.
- The same bridge now has explicit evidence that oversized decoded SR payload rejection is propagated without mapping payload bytes into the internal decoded-message output.
- Both reject paths clear stale decoded-message output.
- Existing receiver mismatch, sender mismatch, future timestamp, stale confirmed timestamp, unsupported type, null policy, and successful mapping evidence remain unchanged.

## Residual

- This closes identity-bridge propagation evidence for the current timestamp and bounded-payload negative vectors.
- Non-none SR checksum/hash algorithms and CRC-bearing redundancy PDU behavior remain selected-requirement work only.

## Conclusion

Accepted as RaSTA SR identity+timestamp handoff negative-vector propagation evidence.
