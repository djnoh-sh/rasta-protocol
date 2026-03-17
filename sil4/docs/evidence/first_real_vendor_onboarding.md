# First Real Vendor Onboarding

## Document Control

- Document ID: `EVID-027`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 첫 실제 vendor MISRA analyzer finding이 들어왔을 때 어떤 절차로 분류, 기록, review, 보고서 연결을 수행할지 정의한다.

목적은 다음과 같다.

1. illustrative sample에서 실제 운영 절차로 전환한다.
2. 첫 실제 finding이 evidence 구조를 깨지 않고 `subset/severity/deviation/review/report` 체인에 들어가게 한다.
3. vendor tool onboarding을 ad-hoc 대응이 아니라 재현 가능한 절차로 고정한다.

## Scope

- 포함:
  - 첫 실제 vendor finding intake 절차
  - rule mapping, severity classification, deviation 여부 판단
  - evidence/report/review linkage
- 제외:
  - vendor tool 구매/라이선스 절차
  - 전체 rule catalog bulk import
  - CI 상시 통합

## Trigger

다음 조건 중 하나가 충족되면 본 절차를 시작한다.

1. 상용 MISRA analyzer trial 또는 정식 도입 후 첫 finding report 확보
2. vendor rule ID가 포함된 첫 static analysis output 확보
3. vendor tool 결과를 공식 review 대상 evidence로 올릴 필요가 생긴 경우

## Required Inputs

- vendor tool name/version/profile
- raw finding export 또는 report
- finding이 발생한 commit ID
- 대상 파일/위치
- provisional vendor rule ID
- current mapping references
  - `vendor_rule_id_mapping_draft.md`
  - `vendor_rule_matrix_sample.md`
  - `misra_subset_severity.md`
  - `severity_mapping.md`

## Onboarding Procedure

### Step 1. Raw Intake

아래 정보를 원본 그대로 보존한다.

- tool source
- execution date
- commit ID
- raw finding text
- vendor rule ID
- file/location

원본 evidence는 first-run report appendix 또는 별도 archived attachment reference로 연결한다.

### Step 2. Rule Family Mapping

vendor rule ID를 먼저 rule family에 매핑한다.

- Memory
- Initialization
- Control Flow
- Type / Interface
- Defensive Handling
- Style / Maintainability

rule family가 애매하면 가장 좁은 safety-relevant family를 우선 적용하고 review record에서 rationale을 남긴다.

### Step 3. Subset and Severity Classification

rule family를 기준으로 `MISRA-S1~S6` subset과 severity를 결정한다.

- 필요 시 safety path context를 근거로 severity 상향 가능
- 상향 시 review record에 “default vs escalated” 근거를 남긴다

### Step 4. Deviation Decision

finding별로 다음 셋 중 하나를 결정한다.

1. immediate fix
2. temporary deviation
3. disputed classification requiring secondary review

temporary deviation이면 `misra_deviation_log.md`에 실제 entry를 추가한다.

### Step 5. Review Record Creation

최소 하나의 review record를 만든다.

review는 아래를 반드시 포함한다.

- rule mapping 타당성
- subset/severity 타당성
- safety impact
- fix/deviation decision
- follow-up owner와 due condition

### Step 6. Report Update

첫 실제 vendor finding은 다음 report 섹션에 반영한다.

- `Severity Bucket Summary`
- `Subset Bucket Summary`
- `Vendor Rule Bucket Summary`
- `Classification Rationale`
- `Open Vendor Deviations`

### Step 7. Baseline Promotion Decision

첫 실제 finding 처리 후 아래를 결정한다.

1. vendor tool을 reference-only로 유지할지
2. second-tool baseline으로 승격할지
3. CI/manual evidence cycle 어디에 넣을지

## Mandatory Outputs

첫 실제 onboarding이 완료되면 최소 아래 산출물이 있어야 한다.

1. 실제 vendor finding이 포함된 static analysis report 1건
2. 실제 review record 1건
3. deviation log entry 0건 이상
4. vendor rule matrix의 실제 entry 1건 이상
5. roadmap/evidence index 갱신

## Exit Criteria

- 첫 실제 vendor finding이 `rule -> subset -> severity -> report -> review` 체인에 연결됐다.
- deviation이 있으면 `misra_deviation_log.md`에 등록됐다.
- sample/example 문서와 실제 record의 차이가 review에서 설명됐다.
- 이후 반복 적용 가능한 checklist 수준의 절차가 확보됐다.

## Current Decision

1. 첫 실제 onboarding 전까지는 sample/example 문서를 유지한다.
2. 첫 실제 onboarding이 완료되면 `first_vendor_deviation_example.md`는 archived reference로 강등한다.
3. vendor baseline 승격 여부는 첫 실제 finding의 처리 비용과 반복 가능성을 확인한 뒤 결정한다.

## Follow-up Actions

1. 첫 실제 vendor finding 발생 시 본 절차 문서에 실적 링크 추가
2. deviation log에 actual vendor entry 추가
3. vendor-aware static analysis report 1건 생성
