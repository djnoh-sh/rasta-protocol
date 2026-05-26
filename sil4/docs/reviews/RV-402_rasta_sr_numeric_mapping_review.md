# RV-402 RaSTA SR Numeric Mapping Review

## Document Control

- Review ID: `RV-402`
- Date: `2026-05-26`
- Scope: `PDU-PARITY-001C`, `TC-CODEC-039`, `R-006`
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

1. RaSTA SR numeric message type constants now match the repo-source `rasta_conn_type` values.
2. RaSTA DiscReq reason constants now match the repo-source `rasta_disconnect_reason` values.
3. Mapping APIs cover current internal message families for `ConnReq`, `ConnResp`, `Heartbeat`, `Data`, `RetrReq`, and `DiscReq`.
4. `RetrResp` and `RetrData` numeric constants are defined but intentionally rejected until internal message families are added.
5. Disconnect reason mapping is explicit and returns typed unsupported status for non-disconnect operational reasons.

## Verification Position

`TC-CODEC-039` covers numeric constant and mapping behavior. Sequential verification was green:

- `cmake -S sil4 -B /tmp/sil4-build`
- `cmake --build /tmp/sil4-build -j4`
- all `/tmp/sil4-build/rsrx_*_test` unit/integration executables
- `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`
