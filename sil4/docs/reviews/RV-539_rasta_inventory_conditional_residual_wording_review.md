# RV-539 RaSTA Inventory Conditional Residual Wording Review

## Document Control

- Review ID: `RV-539`
- Scope: `INV-RASTA-001`, `R-006`
- Status: `Accepted`
- Date: `2026-06-19`

## Review Summary

The RaSTA normative feature inventory still used some imperative "Add" or "Implement" wording for work that is intentionally conditional on controlled requirement selection or target evidence availability.

This review tightens the inventory wording so open residuals are not misread as immediately unblocked implementation tasks.

## Evidence Reviewed

- `sil4/docs/design/rasta_normative_feature_inventory.md`
- `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
- `sil4/docs/roadmap_status.md`

## Accepted Position

- Non-none checksum/hash calculation remains conditional on selected profile requirement.
- CRC-bearing redundancy behavior remains conditional on selected redundancy CRC option requirement.
- Target timestamp-source refresh policy and hardware acceleration remain target evidence work.
- Unsupported message families and additional disconnect reason mappings remain controlled protocol-scope growth, not unbounded local implementation backlog.

## Verification Position

This is a document-only wording cleanup. Host build/test/cppcheck execution is not required because no code, tests, build configuration, or executable behavior changed.
