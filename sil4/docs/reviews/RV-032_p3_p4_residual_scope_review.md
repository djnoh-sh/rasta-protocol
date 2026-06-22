# Review Record - P3/P4 Residual Scope Reassessment

- Review ID: `RV-032`
- Date: `2026-03-23`
- Scope: `R-001~R-003 residual scope reassessment after closeout wrappers`
- Reviewer: `Project Team`
- Status: `Completed`

## Reviewed Inputs

- `sil4/docs/roadmap_status.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/verification/protocol_context_test_spec_draft.md`
- `sil4/docs/verification/transport_supervisor_test_spec_draft.md`
- `sil4/docs/verification/integration_harness_test_spec_draft.md`
- `sil4/tests/unit/test_rsrx_protocol_context.c`
- `sil4/tests/unit/test_rsrx_transport_supervisor.c`
- `sil4/tests/integration/test_rsrx_session_supervisor_flow.c`

## Review Objective

- closeout wrapper와 representative integration 추가 이후 `R-001`, `R-002`, `R-003` 설명이 실제 남은 큰 작업만 가리키는지 점검한다.
- `P3/P4` 다음 우선순위가 현재 residual 무게와 맞는지 재평가한다.

## Findings

1. `R-001`은 더 이상 broad sequencing gap이 아니다.
   - retransmission ordering, steady-state ordering, post-recovery ordering, repeated-gap progression/recovery/post-recovery ordering, sequenced family ordering이 unit matrix로 닫혔다.
   - representative integration도 `DATA`, `HEARTBEAT`, `RETRANSMISSION_REQUEST` family까지 올라왔다.
   - 잔여는 family-level integration parity의 residual에 가깝다.
2. `R-002`도 더 이상 broad runtime loop gap이 아니다.
   - budget scope, send feedback ordering, channel event ordering, timer delegation, poll/pump receive ordering이 unit matrix와 closeout wrapper 수준으로 정리됐다.
   - 남은 일은 queue/backpressure 확장 시 semantics 경계와 `R-004`와의 책임 분리에 더 가깝다.
3. `R-003`은 여전히 가장 무거운 `P4` residual이다.
   - representative closeout은 확보됐지만 richer hysteresis와 long-run stability는 아직 policy-level closing work가 남아 있다.

## Actions Taken

1. roadmap의 `R-001` 설명을 broad sequencing 미완에서 family-level residual로 축소했다.
2. roadmap의 `R-002` 설명을 broad runtime loop 미완에서 retry/runtime feedback residual로 축소했다.
3. roadmap의 `Recommended Next Order`를 `R-003` 우선 기준으로 재정렬했다.
4. `G-P3-P4-Closeout` 목표 문구를 실제 residual scope 기준으로 재정렬했다.

## Residual Major Work

1. `R-003`
   - richer hysteresis와 long-run stability rule을 실제 남은 policy gap 수준으로 더 줄여야 한다.
2. `R-004`
   - queue/backpressure/retry semantics 확장은 아직 minimal bounded queue 모델에 머물러 있다.
3. `R-005`
   - actual CI runtime evidence와 actual vendor evidence set이 아직 비어 있다.
4. `R-001`, `R-002`
   - open이지만, 이제 residual scope는 representative coverage 부족이 아니라 family parity와 queue-boundary semantics 수준이다.

## Conclusion

- `P3/P4`는 현재 broad hardening 단계보다 residual closeout 단계로 읽는 것이 정확하다.
- 앞으로 roadmap은 `R-003`, `R-004`, `R-005`를 상대적으로 더 무거운 open item으로 취급해야 한다.
