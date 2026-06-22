# RV-178 Repetitive Pattern Edit Scope Review

## Summary
- Long repeated pattern sessions increase the chance of edit-scope mistakes more than they increase factual uncertainty.
- The main risk is not loss of intent but oversized mechanical edits that touch adjacent wrapper, registration, or definition-order regions.

## Observed Failure Modes
- Large-range automatic substitution can copy adjacent functions together with the intended function.
- Repeated numeric progression edits can leave off-by-one mistakes in holdoff and remaining counters.
- Moving registration calls ahead of new function definitions can trigger declaration-order build failures.

## Decision
- Repetitive pattern expansion must be performed in narrow edit units:
  - one new function
  - one registration site
  - one document set
- If automatic duplication is used, the added function block must be re-read before build to verify:
  - numeric progression
  - registration inclusion
  - definition order

## Result
- The mitigation is now recorded as a repository work rule rather than left as an informal preference.
- Future threshold-style expansions should therefore be more conservative and easier to audit.
