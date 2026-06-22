# Review Record - RV-477 Confirmed Timestamp Stale Guard

## Scope

- `TC-CODEC-043`
- `R-006 Codec/security`
- V&V comprehensive audit Finding G

## Findings

- V&V Finding G correctly identified that `rsrx_codec_validate_rasta_sr_timestamp_admission()` rejected confirmed timestamps beyond the future boundary but did not reject confirmed timestamps older than the past boundary.
- The codec now applies the same configured past boundary to `uConfirmedTimestamp` and returns `RSRX_CODEC_STATUS_TIMESTAMP_STALE` when the confirmed timestamp is stale.
- `TC-CODEC-043` now includes a confirmed-stale negative vector, keeping timestamp admission behavior symmetric for current and confirmed timestamp window checks.

## Residual

- This closes the portable codec admission defect identified by Finding G.
- Remaining `R-006` residuals are unchanged: non-none checksum behavior only if selected, optional MAC/security extension definition, CRC-bearing redundancy behavior if selected, and broader vendor-oriented security negative vectors.

## Conclusion

Accepted as the corrective action for V&V Finding G in the portable codec baseline.
