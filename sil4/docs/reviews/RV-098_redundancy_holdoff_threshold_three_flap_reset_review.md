# RV-098 Redundancy Holdoff Threshold Three Flap Reset Review

- Scope: `R-003 holdoff threshold 3 flap-reset parity`
- Date: `2026-03-31`
- Reviewer: `Project Team`

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Review Focus

- holdoff threshold `3`에서 flap reset이 들어오면 stable-selection count가 실제로 초기화되는지 점검한다.
- higher threshold parity가 flap-reset semantics와 결합돼도 기존 bypass/selection contract를 바꾸지 않는지 확인한다.

## Findings

1. `TC-CHM-014`는 `failover -> hold -> flap reset -> renewed hold -> renewed hold -> renewed recovery` 경로에서 holdoff `3`의 flap-reset parity를 unit으로 고정한다.
2. `TC-INT-124`는 같은 정책을 session/supervisor 경계까지 올려 flap 이후 restored-again 구간에서 다시 세 번째 recovery refresh에서만 primary 복귀가 일어남을 확인한다.
3. 이번 배치는 새로운 redundancy mode를 추가하지 않고 existing hysteresis contract의 higher-threshold flap-reset parity만 닫는다.

## Decision

- `R-003`의 richer hysteresis threshold residual은 broad gap이 아니라 threshold growth와 reset interaction의 next-step refinement로 더 좁아졌다.
- representative unit/integration parity 추가는 타당하다.
