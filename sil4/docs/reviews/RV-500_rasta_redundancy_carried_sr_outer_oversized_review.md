# Review Record - RV-500 RaSTA Redundancy Carried SR Outer Oversized

## Scope

- `TC-CODEC-050`
- `R-006 Codec/security`
- RaSTA redundancy carried SR no-checksum decode bridge

## Findings

- The carried SR decode bridge now has explicit evidence that an outer redundancy PDU declaring a carried packet larger than the bounded SR frame capacity returns `RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE`.
- The bridge preserves the outer redundancy decoder status instead of collapsing the bounded-capacity rejection into a generic bridge failure.
- The stale SR decoded-packet output is cleared on the outer oversized carried-packet path.
- Existing round-trip, outer malformed, and inner malformed status-preservation evidence remains unchanged.

## Residual

- This closes the option A no-CRC carried-SR bridge outer oversized carried-packet negative vector.
- CRC-bearing redundancy PDU behavior remains selected-requirement work only.

## Conclusion

Accepted as RaSTA redundancy carried SR outer oversized status-preservation evidence.
