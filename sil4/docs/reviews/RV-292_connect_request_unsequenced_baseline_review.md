# Review Record - Connect Request Unsequenced Baseline Decision

## Document Control

- Review ID: `RV-292`
- Scope: `CONNECT_REQUEST` unsequenced baseline decision`
- Status: `Accepted`
- Reviewer: `Codex`
- Review Date: `2026-04-29`

## Reviewed Artifacts

- `sil4/tests/unit/test_rsrx_protocol_context.c`
- `sil4/docs/design/lld/protocol_context_lld_draft.md`
- `sil4/docs/verification/protocol_context_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Review Focus

- current baseline이 `CONNECT_REQUEST`를 inbound admission trigger로만 유지하는지
- 이 decision이 unsequenced family matrix의 간접 해석이 아니라 explicit test/spec wording으로 고정됐는지

## Findings

- `vTestConnectRequestRemainsUnsequencedBaseline` now verifies that arbitrary sequence/confirmation values on inbound `CONNECT_REQUEST` do not mutate tracking state and that the following sequenced inbound `CONNECT_RESPONSE` still admits sequence `1`.
- LLD and test spec now state explicitly that the current baseline does not treat `CONNECT_REQUEST` as the start of the sequenced inbound family.
- roadmap residual wording can now remove `CONNECT_REQUEST` sequencing baseline decision from the open portion of `R-001`.

## Conclusion

- the current baseline decision for inbound `CONNECT_REQUEST` handling is now explicit, test-backed, and review-linked.
- any future move to sequence inbound `CONNECT_REQUEST` should be treated as a new parity-policy change rather than a latent ambiguity in the present baseline.
