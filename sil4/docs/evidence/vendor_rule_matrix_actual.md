# Vendor Rule Matrix Actual

## Document Control

- Document ID: `EVID-CI-044`
- Version: `0.1.0`
- Status: `Stub`
- Owner: `Project Team`
- Last Updated: `2026-03-25`

## Purpose

이 문서는 `EVS-006`을 실제 운영형 artifact로 수용하기 위한 actual vendor rule matrix다.

현재는 first actual vendor finding이 아직 없으므로 구조만 고정한다. 첫 actual finding이 확보되면 아래 entry를 sample이 아니라 운영값으로 채운다.

## Relationship To Other Artifacts

- first execution checklist:
  - `first_actual_vendor_evidence_set_checklist.md`
- first actual finding report stub:
  - `reports/first_actual_vendor_finding_set_stub.md`
- sample reference:
  - `vendor_rule_matrix_sample.md`
- first actual entry sample:
  - `first_actual_vendor_rule_entry_sample.md`

## Matrix Fields

| Field | Description |
| --- | --- |
| Tool Source | vendor tool 식별자 |
| Workflow Run ID | finding을 생성한 workflow 또는 capture run id |
| Vendor Rule ID | tool-specific rule number |
| Vendor Rule Family | memory / initialization / control-flow / type-interface / defensive / style |
| Subset ID | `MISRA-S1`~`MISRA-S6` |
| Severity | `Critical/High/Medium/Low/Info` |
| File | finding 대상 파일 |
| Location | line/column 또는 equivalent locator |
| Initial Decision | `Fix Required` / `Deviation Review` / `False Positive Review` |
| Report Link | runtime report section 또는 report id |
| Review Link | review record id |
| Tracking Link | deviation log entry 또는 fix tracking reference |
| Status | `Open` / `Mitigated` / `Closed` |

## Operational Entries

| Tool Source | Workflow Run ID | Vendor Rule ID | Vendor Rule Family | Subset ID | Severity | File | Location | Initial Decision | Report Link | Review Link | Tracking Link | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` | `TBD` |

## Completion Rule

다음이 모두 채워지면 `EVS-006`이 완료된 것으로 본다.

1. 실제 vendor rule id와 file/location이 기록된다.
2. `EVS-004` report link와 `EVS-005` review link가 연결된다.
3. deviation 또는 fix tracking reference가 연결된다.
4. audit trail에서 본 문서를 actual matrix artifact로 참조할 수 있다.

## Notes

- 이 문서는 sample matrix를 대체하지 않는다.
- first actual vendor finding 이전에는 `Stub` 상태를 유지한다.
- second actual finding 이후에는 entry를 누적해 operational matrix로 확장한다.
