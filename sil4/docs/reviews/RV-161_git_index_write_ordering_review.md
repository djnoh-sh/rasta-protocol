# RV-161 Git Index Write Ordering Review

## Scope
- `git` write 단계도 verification ordering과 같은 수준의 순차 규칙으로 고정한다.

## Observation
- `build/test` overlap과 별개로 `git add`와 `git commit`을 병렬 실행하면 `.git/index.lock` 충돌이 재현될 수 있다.
- 이 문제는 제품 코드 결함이 아니라 single-writer 전제인 git index를 병렬로 건드린 절차 문제다.

## Judgment
- 저장소 변경관리도 verification과 마찬가지로 phase ordering discipline이 필요하다.
- 특히 index를 쓰는 git 명령은 helper 병렬화 대상이 아니라 순차 실행 대상으로 강제해야 한다.

## Decision
- `SIL4_REIMPLEMENTATION_RULES.md`의 change management 규칙에 git index write ordering rule을 추가한다.
- index를 쓰는 git 명령은
  - `git add`
  - staged diff 확인
  - `git commit`
  순서로만 진행한다.
- `index.lock` 오류가 나면 먼저 동시 실행 여부와 살아있는 git 프로세스를 확인한다.

## Result
- 앞으로의 git index lock 문제는 저장소 이상보다 절차 위반 여부를 먼저 확인할 수 있다.
- 변경관리 단계도 verification ordering과 같은 수준의 순차 규율을 갖게 된다.
