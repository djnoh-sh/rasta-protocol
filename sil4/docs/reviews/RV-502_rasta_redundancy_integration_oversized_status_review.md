# Review Record - RV-502 RaSTA Redundancy Integration Oversized Status

## Scope

- `TC-INT-210`
- `R-006 Codec/security`
- RaSTA redundancy-carried SR integration runtime

## Findings

- The integration harness now exercises an oversized carried-packet redundancy PDU through the fake transport receive boundary, supervisor redundancy SR runtime, and real codec helper path.
- The integration path rejects the frame as `RSRX_SUPERVISOR_STATUS_DECODE_FAILED` and preserves `RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE` in the supervisor report.
- The rejected oversized frame does not increase processed-frame count and does not emit an additional application callback.
- Existing admitted redundancy-carried SR and wrong-envelope direct SR integration evidence remains unchanged.

## Residual

- This closes the host integration evidence for option A no-CRC carried-SR oversized decode-status propagation.
- CRC-bearing redundancy PDU behavior remains selected-requirement work only.

## Conclusion

Accepted as RaSTA redundancy SR integration oversized carried-packet status propagation evidence.
