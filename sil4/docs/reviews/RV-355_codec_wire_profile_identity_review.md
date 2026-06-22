# RV-355 Codec Wire Profile Identity Review

- Review ID: `RV-355`
- Date: `2026-05-08`
- Scope: codec wire profile identity and security-field byte-size boundary.

## Findings

- `rsrx_codec_wire_profile_t` now reports profile id and profile version in addition to header, payload, and frame-size bounds.
- Default and CRC32 profiles are distinguishable through `D_RSRX_CODEC_WIRE_PROFILE_DEFAULT` and `D_RSRX_CODEC_WIRE_PROFILE_CRC32`.
- The default profile explicitly reports zero CRC/MAC/timestamp field byte sizes and absent flags.
- The CRC32 profile explicitly reports `D_RSRX_CODEC_CRC_BYTES` while keeping MAC/timestamp field byte sizes at zero.
- `TC-CODEC-024` and `TC-CODEC-027` now verify the profile identity/version and field byte-size boundary, so deployment evidence does not need to infer selected PDU shape from callback addresses alone.

## Residual

- Current profile identity evidence reduces PDU-boundary ambiguity for default and CRC32 profiles.
- Actual MAC/timestamp/PDU parity remains future codec-security implementation growth and is not claimed by this profile metadata.
