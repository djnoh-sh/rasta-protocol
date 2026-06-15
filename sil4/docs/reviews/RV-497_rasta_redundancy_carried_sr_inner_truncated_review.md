# Review Record - RV-497 RaSTA Redundancy Carried SR Inner Truncated

## Scope

- `TC-CODEC-050`
- `R-006 Codec/security`
- RaSTA redundancy carried SR no-checksum decode bridge

## Findings

- The carried SR decode bridge now has explicit evidence that an inner SR packet declaring more bytes than the carried packet contains returns `RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD`.
- The bridge preserves the inner SR decoder status instead of collapsing it into a generic outer redundancy failure.
- The stale SR decoded-packet output is cleared on the inner truncated-payload path.
- Existing round-trip, null input, inner unsupported type, and outer malformed frame evidence remain unchanged.

## Residual

- This closes an option A no-CRC redundancy-carried SR decode bridge negative vector.
- CRC-bearing redundancy PDU behavior remains selected-requirement work only.

## Conclusion

Accepted as RaSTA redundancy carried SR inner decoder status-preservation evidence.
