# Review Record - RV-494 RaSTA SR Confirmed Timestamp Causal Order

## Scope

- `TC-CODEC-043`
- `TC-CODEC-044`
- `R-006 Codec/security`
- RaSTA SR timestamp admission and timestamp-admitted handoff mapping

## Findings

- The timestamp admission boundary now rejects an SR packet when `confirmed_timestamp` is greater than the packet `timestamp`.
- The reject path returns `RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE`, preserving the existing timestamp taxonomy without adding a new public status.
- The timestamp-admitted handoff bridge observes the same rejection before message mapping and clears stale decoded-message output.
- Existing zero, stale, regressed, future, boundary overflow/underflow, and null-argument timestamp checks remain unchanged.

## Residual

- This closes a portable SR timestamp causal-order edge case.
- Target timestamp-source binding and target replay/fault-injection evidence remain `R-008`/`R-009` target evidence work.
- Non-none SR checksum/hash algorithms and CRC-bearing redundancy PDU behavior remain selected-requirement work only.

## Conclusion

Accepted as a portable RaSTA SR timestamp admission hardening step.
