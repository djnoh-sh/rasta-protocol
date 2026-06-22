# Outbound Queue Policy Evidence Note

## Report Information

- Report ID: `EVID-OUT-001`
- Date: `2026-03-17`
- Commit ID: `Pending`
- Author: `Codex`
- Reviewer: `TBD`
- Category: `Design and Verification Evidence`

## Scope

- Target Artifacts:
  - `sil4/docs/design/lld/outbound_application_data_lld_draft.md`
  - `sil4/docs/verification/outbound_application_data_test_spec_draft.md`
  - `sil4/docs/verification/integration_harness_test_spec_draft.md`
  - `sil4/src/rsrx_api.c`
  - `sil4/src/rsrx_platform_adapters.c`
  - `sil4/src/rsrx_transport_supervisor.c`
- Related Requirements:
  - `FR-003`
  - `IF-001`
- Trigger:
  - bounded outbound queue policy를 design decision과 verification evidence로 고정

## Policy Baseline

- 현재 outbound application send 정책은 `outstanding 1 + deferred 1`의 bounded queue 모델이다.
- 허용 규칙:
  - outstanding가 없으면 즉시 send
  - outstanding가 있으면 second request는 deferred slot에 enqueue
  - outstanding와 deferred가 모두 차 있으면 third request는 reject
- clear 규칙:
  - valid inbound message
  - correlated transport feedback
  - explicit clear path
- reject 규칙:
  - overflow reject는 synthetic API report와 diagnostic record를 남긴다.
  - repeated busy reject는 configured threshold에서 `WARN_REJECTED_EVENT -> ERROR_INTERFACE`로 승격할 수 있다.

## Rationale

1. 동적 메모리와 unbounded backlog를 피한다.
2. transport feedback correlation과 sequencing context를 깨지 않고 backpressure를 제공한다.
3. overflow를 무음 drop이 아니라 API callback, diagnostics, telemetry로 관찰 가능하게 남긴다.
4. 이후 multi-depth queue를 도입하더라도 현재 단일-slot 정책을 명시적 baseline으로 유지할 수 있다.

## Evidence Mapping

| Evidence Area | Artifact / Test | Coverage |
| --- | --- | --- |
| Unit design intent | `LLD-013` | bounded queue, overflow reject, threshold escalation 규칙 |
| Unit verification | `TC-OUT-005` | second send enqueue, third send reject |
| Unit verification | `TC-OUT-006` | deferred dispatch after clear |
| Unit verification | `TC-OUT-007` | accepted/busy/clear telemetry |
| Unit verification | `TC-OUT-008` | repeated busy reject escalation |
| Integration verification | `TC-INT-014` | deferred queue telemetry after correlated `SEND_COMPLETED` |
| Integration verification | `TC-INT-015` | overflow reject API/diagnostic/telemetry correlation |

## Execution Record

- Build:
  - `cmake --build /tmp/sil4-build -j4`
- Tests:
  - `/tmp/sil4-build/rsrx_api_test`
  - `/tmp/sil4-build/rsrx_platform_adapters_test`
  - `/tmp/sil4-build/rsrx_transport_supervisor_test`
  - `/tmp/sil4-build/rsrx_session_supervisor_flow_test`
- Analyzer:
  - `cppcheck --enable=warning,style,performance,portability --std=c11 --force --inline-suppr sil4/include sil4/src sil4/tests/unit sil4/tests/integration`

## Residual Risks

| Risk ID | Description | Current Handling |
| --- | --- | --- |
| OUT-RSK-001 | queue depth가 1-slot deferred로 제한돼 burst traffic 대응이 단순하다 | policy를 bounded baseline으로 명시, overflow telemetry로 관찰 |
| OUT-RSK-002 | overflow reject는 retry scheduling을 포함하지 않는다 | 상위 application/API가 reject를 보고 재시도 정책을 결정 |
| OUT-RSK-003 | threshold escalation은 session-local synthetic diagnostic에 머문다 | 추후 CI/evidence와 system-level reporting linkage 필요 |

## Result

- Result: `Pass with Known Limits`
- Summary:
  - 현재 outbound queue 정책은 구현, unit test, integration test, telemetry, diagnostics 기준으로 일관되게 닫혔다.
  - 다만 multi-depth queue, retry scheduler, system-level escalation linkage는 아직 범위 밖이다.
