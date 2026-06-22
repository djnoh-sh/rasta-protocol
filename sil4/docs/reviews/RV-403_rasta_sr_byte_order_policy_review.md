# RV-403 RaSTA SR Byte-Order Policy Review

## Document Control

- Review ID: `RV-403`
- Date: `2026-05-28`
- Scope: `TC-CODEC-040`, `PDU-PARITY-001D-pre`, `R-006`
- Status: `Accepted`

## Scope Reviewed

- `sil4/include/rsrx_codec.h`
- `sil4/src/rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_codec.c`
- `sil4/tests/unit/test_rsrx_codec_contract.c`
- `sil4/docs/verification/protocol_codec_test_spec_draft.md`
- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/roadmap_status.md`

## Findings

1. The SIL4 RaSTA SR profile now has an explicit fixed big-endian byte-order policy.
2. Public helpers cover 16-bit and 32-bit SR header fields with null-argument rejection.
3. The policy intentionally avoids inheriting the repo-source host-endian behavior.
4. Legacy interoperability with host-endian peers is not claimed by this step and would require deployment-specific golden vectors.

## Verification Position

`TC-CODEC-040` covers the byte-order helper contract. Sequential verification was green:

- `cmake -S sil4 -B /tmp/sil4-build`
- `cmake --build /tmp/sil4-build -j4`
- all `/tmp/sil4-build/rsrx_*_test` unit/integration executables
- `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
