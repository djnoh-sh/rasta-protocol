# RV-157 Verification Execution Ordering Rule Review

## Scope
- build/test/static-analysis 실행 순서를 작업 규칙으로 고정한다.

## Observation
- 최근 작업에서 실제 코드 결함이 아니라 `build`와 test binary 실행이 시간적으로 겹쳐 보이는 순간들이 있었다.
- 이 경우 링크 완료 전 binary를 집어서 false failure처럼 보이는 noise가 섞일 수 있다.

## Judgment
- 이 문제는 개별 커맨드 선택 실수보다 검증 실행 순서가 명시 규칙으로 고정되지 않았기 때문에 반복될 가능성이 있다.
- 따라서 local/CI 공통으로 `configure -> build -> build 종료 확인 -> test -> static analysis` 순서를 규칙화해야 한다.

## Decision
- `SIL4_REIMPLEMENTATION_RULES.md`의 test/workflow 규칙에 verification ordering rule을 추가한다.
- verification helper script에도 같은 원칙을 주석으로 명시한다.
- 이후 검증 실패 해석 전에는 먼저 build 완료 여부를 확인하는 것을 기본 절차로 본다.

## Result
- 앞으로의 검증 실패는
  - 실제 제품 코드/테스트 실패
  - build/test overlap로 인한 false failure
  를 더 명확히 구분할 수 있게 된다.
- 이 저장소의 기본 작업 규율에 verification ordering discipline이 포함된다.
