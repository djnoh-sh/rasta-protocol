# RV-096 Redundancy Holdoff Threshold Four Review

- Scope: `R-003 holdoff threshold > 3 parity`
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

- threshold `3`까지 닫혀 있던 hysteresis parity를 threshold `4`에서도 같은 정책 구조로 유지하는지 점검한다.
- higher threshold가 flap/bypass semantics를 바꾸지 않고 stable-selection count만 한 단계 더 늘리는지 확인한다.

## Findings

1. `TC-CHM-013`은 `failover -> hold -> hold -> hold -> recovery` 경로에서 threshold `4`의 pure holdoff semantics를 unit으로 고정한다.
2. `TC-INT-122`는 같은 정책을 session/supervisor 경계까지 올려 `CHANNEL_UP(primary)` 네 번째에서만 primary 복귀가 발생함을 확인한다.
3. 이번 배치는 새로운 redundancy policy를 추가하지 않고 existing hysteresis contract의 higher-threshold parity만 한 단계 더 확장한다.

## Decision

- `R-003`의 richer hysteresis threshold residual은 broad gap이 아니라 configurable threshold growth parity 쪽의 next-step refinement로 더 좁아졌다.
- representative unit/integration parity 추가는 타당하다.
