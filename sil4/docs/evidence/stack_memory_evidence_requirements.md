# Stack and Memory Evidence Requirements

## Document Control

- Document ID: `EVID-CI-110`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-05-06`

## Purpose

이 문서는 V&V v1.4 Finding 4에서 권고한 stack-bound analysis와 static memory map evidence를 위조 없이 확보하기 위한 요구사항을 정의한다.

본 문서는 증빙 자체가 아니다. 실제 외부 제출 증빙은 tool output, build configuration, review record가 모두 연결된 별도 artifact로 생성해야 한다.

## Required Evidence Artifacts

| Evidence Item | Required Source | Minimum Content | Acceptance Rule |
| --- | --- | --- | --- |
| Stack-bound analysis | compiler stack-usage output, qualified static analyzer output, or target/vendor stack analysis report | tool/version, build flags, analyzed object set, per-function or per-task stack bound, unresolved/recursive call handling | 사람이 수기로 추정한 값만으로는 불가하며, 분석 대상과 tool output을 역추적할 수 있어야 한다 |
| Static memory map | linker map file or target memory allocation report | text/rodata/data/bss/stack/heap region sizes, linker script or memory-region source, build configuration | executable image와 동일한 build configuration에서 생성되어야 한다 |
| Strict warning default-gate decision | CI workflow or release build policy record | whether `RSRX_ENABLE_STRICT_WARNING_HARDENING` is mandatory, conditional, or evidence-only; exception/deviation path | 단순 local clean result가 아니라 CI/release gate policy로 추적 가능해야 한다 |

## Accepted Source Types

- compiler-generated stack usage files, for example GCC `-fstack-usage` output
- linker-generated map files tied to the exact release candidate build
- vendor/static-analysis stack reports with tool version and configuration
- CI workflow logs proving the strict-warning gate policy in effect
- release checklist or safety case appendix that references the generated artifacts

## Not Accepted As Final Evidence

- hand-written stack estimates without a tool output reference
- map excerpts that cannot be tied to the executable build
- local-only warning-hardening runs that are not tied to CI or release policy
- screenshots without run id, commit id, tool version, and artifact path
- placeholder reports, stubs, or templates

## Integration With R-005

These artifacts extend the first actual vendor evidence set:

1. `EVS-009` stack-bound analysis artifact
2. `EVS-010` static memory map artifact
3. `EVS-011` strict-warning CI/release default-gate decision

They do not replace `EVS-003` through `EVS-008`; vendor finding evidence remains separately required.

## Exit Criteria

This evidence family can be considered ready when all of the following are true:

1. the stack-bound artifact references a commit, build configuration, tool version, and generated output path
2. the memory-map artifact references the same build configuration or explicitly explains any difference
3. the strict-warning default-gate decision is recorded in CI/release policy or a reviewed deviation record
4. `first_actual_vendor_evidence_set_execution_tracker.md` links the actual artifacts
5. the final review record states whether each artifact is certification-ready or only internally informative
