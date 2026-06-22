# RV-557 RaSTA SCI/Application Aggregation Scope Decision Review

## Document Control

- Review ID: `RV-557`
- Date: `2026-06-19`
- Scope: SCI/application-message aggregation scope decision and roadmap alignment
- Related Documents:
  - `sil4/docs/design/rasta_sci_application_aggregation_scope_decision.md`
  - `sil4/docs/design/rasta_sr_pdu_wire_profile_draft.md`
  - `sil4/docs/design/rasta_normative_feature_inventory.md`
  - `sil4/docs/roadmap_status.md`

## Review Summary

`SCI-SCOPE-001` converts the previous "scope decision needed" backlog item into an explicit controlled exclusion. The current SIL4 core remains a bounded SR payload byte transport and does not claim SCI-P, SCI-LS, or `RastaMessageData` multi-message aggregation parity.

## Findings

| ID | Finding | Resolution |
| --- | --- | --- |
| RV-557-F1 | SCI/application aggregation was visible as a backlog item but lacked a standalone decision artifact. | Added `SCI-SCOPE-001` with current claim boundary and reopen conditions. |
| RV-557-F2 | The roadmap still listed the item as scope-decision-needed. | Updated `PLAN-001` to mark it scoped out until official/customer input reopens it. |
| RV-557-F3 | The PDU profile excluded aggregation but did not point to a concrete decision document. | Updated `PDU-PARITY-001` wording to reference `SCI-SCOPE-001`. |

## Verification

This is a document-only change. No source or executable test logic was changed.

Required check:

- `git diff --check`

## Review Position

The selected SIL4 SR baseline should continue to treat data as bounded payload bytes. SCI/application aggregation should be implemented only after a controlled reopen packet supplies exact scope, wire layout, vectors, ownership, and target evidence expectations.
