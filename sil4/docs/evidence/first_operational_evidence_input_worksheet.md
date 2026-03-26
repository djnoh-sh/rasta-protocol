# First Operational Evidence Input Worksheet

## Document Control

- Document ID: `EVID-CI-070`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-26`

## Purpose

이 문서는 `R-005` actual execution 직전에 baseline/vendor track에서 실제로 수집해야 하는 입력값을 한 장으로 모은 worksheet다.

helper/runbook/packet 문서가 실행 절차를 담는다면, 본 문서는 다음만 빠르게 확인하게 한다.

1. 지금 어떤 track를 실행하는가
2. 어떤 field를 실제 artifact에서 채워야 하는가
3. 어떤 field가 아직 비어 있는가

## Track A: Baseline Fetch Inputs

| Field | Source | Value |
| --- | --- | --- |
| `report-id` | local naming decision | `TBD` |
| `review-id` | local naming decision | `TBD` |
| `execution-date` | workflow run date | `TBD` |
| `commit-id` | workflow run page | `TBD` |
| `ref-name` | workflow run page | `TBD` |
| `run-id` | workflow run page | `TBD` |
| `trigger-ref` | workflow run page | `TBD` |
| `source-type` | baseline resolve result | `TBD` |
| `source-run-id` | baseline resolve result | `TBD` |
| `workflow-url` | workflow run page | `TBD` |
| `artifact-ref` | workflow artifacts page | `TBD` |
| `resolve-log-ref` | step log | `TBD` |
| `download-log-ref` | step log | `TBD` |
| `materialize-log-ref` | step log | `TBD` |
| `annotate-log-ref` | step log | `TBD` |
| `log-dir` | local extracted artifact dir | `TBD` |

## Track B: Vendor Finding Inputs

| Field | Source | Value |
| --- | --- | --- |
| `report-id` | local naming decision | `TBD` |
| `review-id` | local naming decision | `TBD` |
| `date` | capture/workflow date | `TBD` |
| `tool-source` | vendor analyzer metadata | `TBD` |
| `commit-id` | capture/workflow metadata | `TBD` |
| `ref-name` | capture/workflow metadata | `TBD` |
| `run-id` | capture/workflow metadata | `TBD` |
| `job-name` | capture/workflow metadata | `TBD` |
| `trigger-ref` | capture/workflow metadata | `TBD` |
| `artifact-name` | vendor export package | `TBD` |
| `tool-version` | vendor analyzer metadata | `TBD` |
| `raw-evidence-type` | raw export metadata | `TBD` |
| `raw-evidence-location` | secured reference | `TBD` |
| `export-format` | raw export metadata | `TBD` |
| `capture-timestamp` | raw export metadata | `TBD` |
| `reviewer-access-path` | secured reference | `TBD` |
| `vendor-rule-id` | first finding | `TBD` |
| `vendor-rule-family` | mapping/classification | `TBD` |
| `subset-id` | mapping/classification | `TBD` |
| `severity` | mapping/classification | `TBD` |
| `file-path` | first finding | `TBD` |
| `location` | first finding | `TBD` |
| `initial-decision` | review preparation | `TBD` |
| `workflow-url` | capture/workflow page | `TBD` |
| `raw-artifact-ref` | secured reference | `TBD` |
| `vendor-report-ref` | target document path | `TBD` |
| `vendor-review-ref` | target document path | `TBD` |
| `vendor-matrix-ref` | target document path | `TBD` |
| `tracking-ref` | target tracking path | `TBD` |
| `audit-trail-ref` | target document path | `TBD` |

## Shared Decision Fields

| Field | Meaning | Value |
| --- | --- | --- |
| `review-result` | review final result | `TBD` |
| `review-summary` | review summary sentence | `TBD` |
| `tracker-item` | first row to close/update | `TBD` |
| `matrix-status` | actual matrix status | `TBD` |

## Recommended Use

1. worksheet를 먼저 채운다.
2. top-level helper invocation example과 대조한다.
3. 빈 값이 없어지면 helper 실행으로 넘어간다.

## Notes

- 이 문서는 execution packet/runbook를 대체하지 않는다.
- 이 문서의 목적은 helper invocation 직전 입력 누락을 줄이는 것이다.
