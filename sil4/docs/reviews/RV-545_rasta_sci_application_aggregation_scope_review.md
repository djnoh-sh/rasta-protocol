# RV-545 RaSTA SCI/Application Aggregation Scope Review

## Review Metadata

- Review ID: `RV-545`
- Date: `2026-06-19`
- Scope: `PDU-PARITY-001`, `INV-RASTA-001`, `PLAN-001`
- Status: `Accepted`
- Reviewers: `Project Team`

## Summary

`INV-RASTA-001` identifies application data payload/message aggregation as a possible RaSTA parity topic if SCI/application aggregation is in scope. The current selected no-checksum SR host baseline carries bounded payload bytes and does not implement multi-application-message aggregation semantics. The roadmap did not explicitly list that scope decision, which could make the residual less visible than checksum, redundancy CRC, or MAC decisions.

## Accepted Position

1. The current PDU wire profile does not claim SCI/application-message aggregation parity.
2. The current host implementation treats the SR data field as a bounded payload byte sequence.
3. SCI/application aggregation must be opened only by a controlled official/customer scope decision before implementation.
4. This item belongs after checksum and redundancy CRC selection in the current priority order because it is a scope decision, not a discovered defect in the selected host baseline.

## Verification Position

This is a document-only scope clarification. No source or executable test logic changed, so full host verification is not required. `git diff --check` is sufficient.

## Traceability Impact

- `PDU-PARITY-001` now explicitly excludes SCI/application aggregation from the current wire-profile claim.
- `PLAN-001` now lists the aggregation decision as a controlled backlog item.
