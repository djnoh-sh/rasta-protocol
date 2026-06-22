# RV-159 Test Helper Ordering Summary Review

## Scope
- operational test helper script들도 phase marker와 ordering summary를 artifact로 남기게 만든다.

## Observation
- `run_ci_verification.sh`, `run_static_analysis_baseline.sh`는 ordering status와 marker path를 summary artifact로 남긴다.
- 반면
  - `test_operational_artifact_runner.sh`
  - `test_operational_input_pipeline.sh`
  - `test_operational_evidence_helpers.sh`
  는 pass/fail은 제공하지만 phase marker와 ordering summary를 별도 artifact로 남기지 않았다.

## Judgment
- helper path까지 ordering evidence를 통일하려면 smoke script도 실행 순서를 artifact 수준에서 남겨야 한다.
- 이러면 helper 실패를 볼 때도 log 한 줄이 아니라 phase-complete 기준으로 해석할 수 있다.

## Decision
- 세 test helper 모두
  - `summary.md`
  - `summary.env`
  - phase marker
  를 남긴다.
- helper별 phase는 `setup`, `baseline_execute`, `vendor_execute`, 필요 시 `validate`로 고정한다.

## Result
- operational helper chain은 CI/static-analysis helper와 같은 방식으로 ordering evidence를 남긴다.
- local smoke helper도 summary artifact만 보면 어떤 단계까지 성공했는지 바로 읽을 수 있다.
