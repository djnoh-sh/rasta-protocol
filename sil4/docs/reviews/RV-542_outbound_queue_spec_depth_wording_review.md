# RV-542 Outbound Queue Spec Depth Wording Review

## Review Metadata

- Review ID: `RV-542`
- Date: `2026-06-19`
- Scope: `TC-OUT-009`, `TC-OUT-010`, `TC-OUT-012`, `TC-INT-102`, `TC-INT-103`, `TC-INT-116`, `TC-INT-118`, `R-004`
- Status: `Accepted`
- Reviewers: `Project Team`

## Summary

The current queue policy is `outstanding 1 + deferred 12`, but several verification-spec rows still described representative FIFO/clear/telemetry paths as `deferred 3` or `three-slot` behavior. Those rows were not intended to claim a smaller configured capacity; they describe first-three-entry representative dispatch paths inside the current twelve-entry deferred queue.

## Accepted Position

1. Full-depth overflow and saturation rows remain expressed as `outstanding 1 + deferred 12`.
2. FIFO, mixed-clear, and telemetry rows that exercise three deferred entries are now described as representative three-entry paths within the twelve-entry capacity.
3. The wording must not imply that the configured deferred capacity has regressed from `12` to `3`.

## Verification Position

This is a document-only verification-spec wording correction. No source or executable test logic changed, so host build/test/cppcheck rerun is not required. `git diff --check` is sufficient for this step.

## Traceability Impact

- `R-004` remains closed for the current `outstanding 1 + deferred 12` policy.
- Representative three-entry FIFO/clear/telemetry coverage remains valid as a path-level check, not as a configured-capacity statement.
- Full-depth overflow evidence remains covered by the existing `deferred 12` rows.
