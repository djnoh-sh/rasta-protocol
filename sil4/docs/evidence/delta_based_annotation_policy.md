# Delta-Based PR Annotation Policy

## Document Control

- Document ID: `EVID-026`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 PR annotation이 단순 snapshot 결과만이 아니라 이전 기준 대비 증가(delta)를 어떻게 해석할지 정의한다.

목적은 다음과 같다.

1. noise가 큰 반복 알림을 줄인다.
2. 실제로 악화된 bucket 변화에만 reviewer attention을 집중시킨다.
3. severity/subset 기반 PR reporting을 운영 규칙으로 고정한다.

## Scope

- 포함:
  - severity bucket delta 규칙
  - subset bucket delta 규칙
  - annotation level escalation 규칙
- 제외:
  - exact persistence backend 결정
  - vendor tool 실제 delta 계산 구현

## Inputs

- current snapshot:
  - `summary.env`
- optional baseline snapshot:
  - previous successful run summary 또는 stored PR baseline
- policy baseline:
  - `pr_annotation_strategy.md`
  - `severity_mapping.md`
  - `tool_specific_misra_mapping.md`

## Delta Definition

delta는 아래처럼 정의한다.

- `delta = current_count - baseline_count`
- baseline이 없으면 delta 판단은 `Not Available`로 둔다.
- baseline이 있을 때만 `increase / equal / decrease`를 결정한다.

## Severity Delta Rules

| Condition | Result |
| --- | --- |
| `Critical delta > 0` | `Blocker` |
| `High delta > 0` | `Blocker` unless approved deviation exactly matches |
| `Medium delta > 0` and no higher delta | `Review Required` |
| only `Low/Info delta > 0` | `Informational` or suppressed by noise threshold |
| all deltas `<= 0` | keep snapshot level, no delta escalation |

## Subset Delta Rules

우선순위 subset은 다음 순서로 본다.

1. `MISRA-S1`
2. `MISRA-S2`
3. `MISRA-S3`
4. `MISRA-S4`
5. `MISRA-S5`
6. `MISRA-S6`

규칙:

- `MISRA-S1~S4 delta > 0`이면 severity delta와 별도로 reviewer note를 추가한다.
- `MISRA-S5 delta > 0`이면 `Medium` 이상 delta와 함께 있을 때만 note를 강화한다.
- `MISRA-S6 delta > 0`만 단독으로 있으면 batch cleanup 대상으로 남길 수 있다.

## Baseline Selection Policy

1. 가능한 경우 같은 PR의 직전 successful annotation snapshot을 baseline으로 사용한다.
2. 그게 없으면 마지막 successful branch baseline을 사용할 수 있다.
3. 둘 다 없으면 `baseline unavailable`로 표시하고 snapshot-only policy로 fallback 한다.

## Output Rules

### Blocker

- 이유: critical/high delta 증가 또는 build/test failure
- 출력:
  - sticky comment 상단에 delta-trigger 명시
  - priority subset delta 명시

### Review Required

- 이유: medium delta 증가 또는 priority subset 증가
- 출력:
  - reviewer action 문장 추가
  - mitigation/deviation 확인 요구

### Informational

- 이유: delta 없음 또는 low-only 증가
- 출력:
  - clean/stable 또는 low-risk drift만 기록

## Noise Threshold Baseline

현재 baseline에서는 아래를 권장한다.

- `Low delta <= 2` 단독 증가는 sticky comment에 요약만 남기고 blocker로 승격하지 않는다.
- 동일 category의 반복 low-only 증가는 batch cleanup backlog로 넘긴다.
- `Medium` 이상은 noise threshold 적용 대상이 아니다.

## Current Decision

1. delta policy는 문서로 먼저 고정한다.
2. 실제 helper 구현은 baseline persistence 경로가 정해진 뒤 추가한다.
3. baseline이 없는 경우 snapshot-only annotation을 정상 fallback으로 허용한다.

## Follow-up Actions

1. baseline persistence source 결정
2. helper script에 delta 계산 추가
3. sticky comment format에 delta section 반영
