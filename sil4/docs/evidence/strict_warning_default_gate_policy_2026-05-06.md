# Strict-Warning Default-Gate Policy

## Document Control

- Document ID: `EVID-CI-118`
- Version: `0.1.0`
- Status: `Complete`
- Owner: `Project Team`
- Last Updated: `2026-05-06`

## Purpose

이 문서는 `RSRX_ENABLE_STRICT_WARNING_HARDENING`의 CI/release default-gate 정책을 기록한다.

## Decision

Selected policy: `Evidence-only`.

The strict warning profile remains a periodic evidence run rather than a mandatory CI/release blocking gate.

## Rationale

1. `SA-REP-006` showed that the current codebase builds cleanly with the strict profile in the evaluated GCC environment.
2. The strict profile adds `-Wconversion` and `-Wsign-conversion` only for GNU/Clang.
3. These diagnostics are compiler-sensitive and can change across compiler versions without a project behavior change.
4. The default CI gate already blocks baseline compiler warnings through `-Wall -Wextra -Werror`.

## Required Cadence

Run or refresh the strict warning evidence when any of the following occurs:

1. release-candidate evidence package preparation
2. compiler major-version change
3. CI runner image/toolchain change
4. broad public API, codec, transport supervisor, or platform adapter type/signature changes
5. external V&V requests an updated warning-hardening sample

## References

- CMake option: `RSRX_ENABLE_STRICT_WARNING_HARDENING`
- Evaluation report: `reports/static_analysis_report_2026-04-29_sa6_strict_warning_hardening.md`
- Review record: `../reviews/RV-331_strict_warning_default_gate_policy_review.md`

## Tracker Impact

`EVS-011` is closed by this reviewed policy decision.
