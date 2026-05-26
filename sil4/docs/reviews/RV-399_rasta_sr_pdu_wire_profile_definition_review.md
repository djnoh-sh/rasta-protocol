# RV-399 RaSTA SR PDU Wire Profile Definition Review

## Document Control

- Review ID: `RV-399`
- Date: `2026-05-26`
- Scope: `PDU-PARITY-001`, `R-006`
- Status: `Accepted with residual follow-up`

## Scope Reviewed

- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/design/rasta_normative_feature_inventory.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. The current SIL4 codec header remains a representative 16-byte skeleton and must not be described as RaSTA SR wire-layout compatible.
2. The repo-source RaSTA SR packet layout is now captured as a 28-byte fixed header followed by data and a configurable safety code/checksum.
3. RaSTA numeric message type values are explicitly identified as required parity inputs for later encode/decode work.
4. The existing repo-source `shortToBytes` and `longToBytes` helpers are host-endian dependent. SIL4 implementation must close the byte-order decision before claiming wire compatibility.
5. Checksum/hash, timestamp validation, redundancy CRC/PDU parity, and MAC/security-extension work remain separate residuals and are not implied by this profile definition.

## Accepted Follow-up

| Follow-up | Required Outcome |
| --- | --- |
| `PDU-PARITY-001A` | Add RaSTA SR profile constants/reporting metadata without changing current encode/decode behavior |
| `PDU-PARITY-001B` | Add explicit RaSTA SR encode/decode contract structures |
| `PDU-PARITY-001C` | Add RaSTA numeric type and disconnect reason mapping tests |
| `PDU-PARITY-001D` | Implement no-checksum SR PDU encode/decode after byte-order policy is closed |
| `PDU-PARITY-001E` | Add selected SR checksum/hash profiles or explicit unsupported-profile rejects |
| `PDU-PARITY-001F` | Integrate timestamp and confirmed-timestamp admission checks |

## Verification Position

This is a documentation-only review. No source, test, build, or static-analysis behavior changed, so no build/test/cppcheck rerun is required for this review item.
