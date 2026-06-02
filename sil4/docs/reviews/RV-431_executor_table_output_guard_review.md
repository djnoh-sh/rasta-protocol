# Review Note - RV-431 Executor Table Output Guard

## Scope

- `rsrx_platform_adapter_build_executor_table`
- `TC-PA-015`

## Change Summary

- The platform adapter executor-table builder now clears a valid output table pointer at function entry.
- Each executor slot is reset to:
  - `pvContext = NULL`
  - `pfDispatch = NULL`
- A unit test covers an invalid external executor with a stale output table pre-populated before the call.

## Safety Rationale

- Executor table build failure must not leave an earlier dispatch binding visible to the caller.
- Clearing the table before validation prevents stale transport, timer, diagnostics, API, application, or lifecycle callbacks from being reused after an invalid platform binding attempt.
- The behavior aligns the executor table boundary with the adapter output-guard policy established by `RV-429` and `RV-430`.

## Verification

- Covered by `TC-PA-015` in `test_rsrx_platform_adapters.c`.
- Full host verification was required because production code and tests changed.
