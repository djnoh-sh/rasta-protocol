# Review Record - RV-481 Invalid Message Numeric Mapping Clear

## Scope

- `TC-CODEC-039`
- `R-006 Codec/security`
- RaSTA SR outbound numeric mapping boundary

## Findings

- `rsrx_codec_map_message_type_to_rasta_sr_type()` already clears the mapped wire type before dispatching supported outbound mappings.
- `TC-CODEC-039` now verifies that `RSRX_MESSAGE_TYPE_INVALID` returns `RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE` and clears a stale mapped wire type to `0`.
- This complements the existing `DIAGNOSTIC`, `RetrResp`, and `RetrData` unsupported mapping output-clear evidence.

## Residual

- No new codec behavior is claimed; this is regression evidence for the current unsupported outbound boundary.
- Broader `R-006` residuals remain unchanged: selected non-none checksum behavior, optional MAC/security extension definition, CRC-bearing redundancy behavior if selected, and vendor-oriented security negative vectors.

## Conclusion

Accepted as numeric mapping boundary evidence for invalid outbound message type output clear behavior.
