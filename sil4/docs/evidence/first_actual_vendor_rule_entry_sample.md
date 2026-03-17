# First Actual Vendor Rule Entry Sample

## Document Control

- Document ID: `EVID-031`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 첫 실제 vendor finding이 들어왔을 때 vendor rule matrix, deviation log, report, review에 어떤 식으로 실제 entry를 남길지 구체적인 샘플을 제공한다.

목적은 다음과 같다.

1. 기존 illustrative matrix sample을 실제 운영 입력 수준으로 내린다.
2. 첫 실제 finding onboarding 절차가 어떤 레코드 세트로 끝나야 하는지 보여준다.
3. evidence/review/audit trail 작성자가 동일한 field set을 공유하도록 만든다.

## Scope

- 포함:
  - first actual vendor matrix entry sample
  - first actual deviation decision sample
  - first actual report linkage sample
- 제외:
  - 실제 vendor tool output 진위
  - 실제 line number 확정
  - 실제 CI integration 결과

## Relationship To Other Artifacts

- onboarding procedure:
  - `first_real_vendor_onboarding.md`
- matrix format baseline:
  - `vendor_rule_matrix_sample.md`
- deviation example:
  - `first_vendor_deviation_example.md`
- mapping draft:
  - `vendor_rule_id_mapping_draft.md`

## Sample Scenario

- Tool Source: `vendor-misra-tool`
- Vendor Rule ID: `VND-TYPE-014`
- Rule Family: `Type / Interface`
- Candidate File: `sil4/src/rsrx_transport_supervisor.c`
- Candidate Theme: protocol field narrowing conversion on decoded sequence path

이 시나리오는 첫 실제 finding을 다루는 구조를 보여주기 위한 운영형 샘플이다.

## Sample Actual Matrix Entry

| Field | Value |
| --- | --- |
| Tool Source | `vendor-misra-tool` |
| Vendor Rule ID | `VND-TYPE-014` |
| Vendor Rule Family | `Type / Interface` |
| Subset ID | `MISRA-S4` |
| Severity | `High` |
| Finding Theme | narrowing conversion on protocol field |
| Typical Trigger | cast or assignment from wider decoded integer to narrower protocol state field |
| Default Action | fix before merge or formal deviation review |
| Deviation Eligibility | 제한적 가능, interface integrity review 필수 |
| Report Section | `High-Risk Findings`, `Classification Rationale`, `Vendor Rule Bucket Summary` |

## Sample Actual Deviation Decision

현재 샘플에서는 default decision을 다음처럼 둔다.

- Initial Decision: `Fix Required`
- Deviation Default: `Not Preferred`
- Escalation Reason:
  - interface integrity path
  - protocol sequence/confirmation semantics에 직접 영향 가능

즉, 첫 actual entry는 되도록 “편차 허용 가능”이 아니라 “수정 우선” 규칙을 보여주는 쪽으로 둔다.

## Sample Report Entry

### High-Risk Findings

| Rule ID | Subset | Severity | File | Status |
| --- | --- | --- | --- | --- |
| `vendor:VND-TYPE-014` | `MISRA-S4` | `High` | `sil4/src/rsrx_transport_supervisor.c` | `Open - Fix Required` |

### Classification Rationale

- `Type / Interface` family이므로 기본 subset은 `MISRA-S4`
- decoded sequence/confirmation handling은 transport supervisor와 protocol context의 interface integrity와 직접 연결됨
- 따라서 severity는 default `High`를 유지하고 하향하지 않는다

### Vendor Rule Bucket Summary

| Family | Count |
| --- | --- |
| Type / Interface | `1` |

## Sample Review Focus

review는 아래를 확인한다.

1. `VND-TYPE-014 -> MISRA-S4 -> High` 매핑이 타당한가
2. 문제 지점이 실제로 protocol integrity path인가
3. 즉시 수정이 가능한가
4. 편차 없이 닫는 것이 현실적인가

## Sample Deviation Log Handling

이 샘플은 “편차를 만들지 않는 첫 actual entry”를 기본으로 한다.

즉, deviation log는 아래처럼 유지될 수 있다.

- new vendor finding recorded
- deviation entry not created
- review record only

이는 “vendor finding이 항상 deviation으로 가는 것은 아니다”라는 운영 원칙을 명확히 한다.

## Current Decision

1. 첫 actual vendor rule entry 샘플은 `High` / `MISRA-S4` / `Fix Required` 사례로 둔다.
2. 첫 actual vendor onboarding에서 반드시 deviation이 필요하다고 가정하지 않는다.
3. 첫 actual entry는 review와 report linkage를 우선 보여주고, deviation은 필요 시에만 추가한다.

## Follow-up Actions

1. 첫 실제 vendor finding이 생기면 본 샘플과 차이를 기록
2. 실제 line/location과 tool export field를 반영한 entry로 교체
3. 필요 시 second sample로 deviation-required case 추가
