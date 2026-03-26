# RV-093 Redundancy Holdoff Threshold Parity Review

- Scope: `R-003 holdoff threshold > 2 parity`
- Date: `2026-03-26`
- Reviewer: `Project Team`

## Inputs Reviewed

- `sil4/tests/unit/test_rsrx_channel_manager.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`
- `sil4/docs/verification/channel_manager_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Review Focus

- holdoff threshold `2` 기준으로 닫혀 있던 hysteresis coverage를 threshold `3`에서도 같은 정책 구조로 유지하는지 점검한다.
- higher threshold가 bypass/flap semantics를 바꾸지 않고 stable-selection count만 늘리는지 확인한다.

## Findings

1. `TC-CHM-012`는 `failover -> hold -> hold -> recovery` 경로에서 threshold `3`의 pure holdoff semantics를 unit으로 고정한다.
2. `TC-INT-121`는 같은 정책을 session/supervisor 경계까지 올려 `CHANNEL_UP(primary)` 세 번째에서만 primary 복귀가 발생함을 확인한다.
3. 이번 배치는 새로운 redundancy policy를 추가하지 않고 existing hysteresis contract의 higher-threshold parity만 닫는다.

## Decision

- `R-003`의 richer hysteresis threshold residual은 broad gap이 아니라 next-step policy refinement 쪽으로 더 좁아졌다.
- representative unit/integration parity 추가는 타당하다.
