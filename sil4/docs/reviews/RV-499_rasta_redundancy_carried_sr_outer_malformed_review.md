# Review Record - RV-499 RaSTA Redundancy Carried SR Outer Malformed

## Scope

- `TC-CODEC-050`
- `R-006 Codec/security`
- RaSTA redundancy carried SR no-checksum decode bridge

## Findings

- The carried SR decode bridge now has explicit evidence that outer redundancy malformed frames preserve their typed outer decoder status.
- Covered outer statuses are short header, length mismatch, truncated payload, trailing bytes, non-frame event, invalid channel, and reserved-header non-zero.
- The stale SR decoded-packet output is cleared before each outer-layer rejection.
- Existing round-trip and inner SR malformed status-preservation evidence remains unchanged.

## Residual

- This closes the option A no-CRC carried-SR bridge outer malformed status-preservation matrix.
- CRC-bearing redundancy PDU behavior remains selected-requirement work only.

## Conclusion

Accepted as RaSTA redundancy carried SR outer malformed status-preservation evidence.
