# Review Record - RV-498 RaSTA Redundancy Carried SR Inner Trailing

## Scope

- `TC-CODEC-050`
- `R-006 Codec/security`
- RaSTA redundancy carried SR no-checksum decode bridge

## Findings

- The carried SR decode bridge now has explicit evidence that an inner SR packet declaring fewer bytes than the carried packet contains returns `RSRX_CODEC_STATUS_TRAILING_BYTES`.
- The bridge preserves the inner SR decoder status instead of collapsing it into a generic outer redundancy failure.
- The stale SR decoded-packet output is cleared on the inner trailing-payload path.
- Existing round-trip, null input, inner unsupported type, inner truncated payload, and outer malformed frame evidence remain unchanged.

## Residual

- This closes an option A no-CRC redundancy-carried SR decode bridge trailing-data negative vector.
- CRC-bearing redundancy PDU behavior remains selected-requirement work only.

## Conclusion

Accepted as RaSTA redundancy carried SR inner trailing status-preservation evidence.
