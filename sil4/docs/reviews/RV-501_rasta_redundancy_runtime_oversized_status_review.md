# Review Record - RV-501 RaSTA Redundancy Runtime Oversized Status

## Scope

- `TC-SUP-077`
- `R-006 Codec/security`
- RaSTA redundancy-carried SR supervisor runtime

## Findings

- The redundancy SR runtime path now has explicit unit evidence that an oversized carried-packet redundancy PDU is rejected as `RSRX_SUPERVISOR_STATUS_DECODE_FAILED`.
- The supervisor report preserves the codec status `RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE`.
- The rejected oversized frame does not increase processed-frame count and does not emit an additional application callback.
- The legacy codec port remains bypassed for the selected redundancy SR runtime path.

## Residual

- This closes the host supervisor propagation evidence for the option A no-CRC carried-SR oversized decode status.
- CRC-bearing redundancy PDU behavior remains selected-requirement work only.

## Conclusion

Accepted as RaSTA redundancy SR runtime oversized carried-packet status propagation evidence.
