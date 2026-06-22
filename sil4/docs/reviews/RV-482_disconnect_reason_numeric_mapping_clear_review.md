# Review Record - RV-482 Disconnect Reason Numeric Mapping Clear

## Scope

- `TC-CODEC-039`
- `R-006 Codec/security`
- RaSTA SR disconnect reason mapping boundary

## Findings

- `rsrx_codec_map_reason_to_rasta_disconnect_reason()` already clears the mapped RaSTA reason before dispatching supported disconnect mappings.
- `TC-CODEC-039` now verifies that an out-of-range internal reason code returns `RSRX_CODEC_STATUS_UNSUPPORTED_REASON` and clears a stale mapped RaSTA reason to `0`.
- This complements the existing non-disconnect reason and null-output guard evidence for the same mapping API.

## Residual

- No new disconnect policy is claimed; this is regression evidence for the current unsupported reason-code boundary.
- Broader `R-006` residuals remain unchanged: selected non-none checksum behavior, optional MAC/security extension definition, CRC-bearing redundancy behavior if selected, and vendor-oriented security negative vectors.

## Conclusion

Accepted as numeric mapping boundary evidence for unsupported disconnect reason output clear behavior.
