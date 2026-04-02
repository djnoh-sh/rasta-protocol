# RV-158 Verification Phase Marker Enforcement Review

## Scope
- verification script가 build/test/static-analysis 순서를 실제로 강제하도록 만든다.

## Observation
- 문서 규칙만으로는 local helper 실행이나 ad-hoc verification에서 순서 위반이 다시 섞일 가능성이 남는다.
- 특히 build 완료 여부를 사람이 눈으로만 해석하는 방식은 false failure 해석 비용을 다시 만들 수 있다.

## Judgment
- verification ordering rule은 문서와 습관 수준이 아니라 script prerequisite 수준까지 내려가야 재발 가능성을 실질적으로 줄일 수 있다.

## Decision
- 공통 helper `verification_sequence_common.sh`를 추가한다.
- helper는
  - phase marker reset
  - phase completion marker 생성
  - prerequisite phase 확인
  - test executable readiness 확인
  를 제공한다.
- `run_ci_verification.sh`, `run_static_analysis_baseline.sh`는 이 helper를 사용해 순서를 강제한다.
- `run_ci_verification.sh`, `run_static_analysis_baseline.sh` summary artifact는 phase marker path와 verification ordering status를 함께 남긴다.

## Result
- verification helper는 이제
  - `configure -> build -> test -> cppcheck`
  또는
  - `build -> cppcheck`
  순서를 marker 기반으로 강제한다.
- build 종료 전 test binary를 집는 종류의 false failure는 helper 경로에서 구조적으로 더 어렵게 된다.
- summary artifact만 봐도 ordering marker가 실제로 생성됐는지 확인할 수 있다.
