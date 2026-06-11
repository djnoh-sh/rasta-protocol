# Review Record - RV-480 RetrData Numeric Mapping Clear

## Scope

- `TC-CODEC-039`
- `R-006 Codec/security`
- RaSTA SR numeric mapping boundary

## Findings

- `RetrResp` and `RetrData` numeric constants are defined but remain unsupported until matching internal message families are introduced.
- `TC-CODEC-039` already verified `RetrResp` unsupported inbound mapping and output clear behavior.
- The test now also verifies `RetrData` unsupported inbound mapping returns `RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE` and clears the mapped message output to `RSRX_MESSAGE_TYPE_INVALID`.

## Residual

- This preserves the current unsupported boundary for future retransmission response/data message-family growth.
- Broader `R-006` residuals remain unchanged: selected non-none checksum behavior, optional MAC/security extension definition, CRC-bearing redundancy behavior if selected, and vendor-oriented security negative vectors.

## Conclusion

Accepted as numeric mapping boundary evidence for unsupported `RetrData` inbound mapping output clear behavior.
