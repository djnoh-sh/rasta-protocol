# Low-Level Design Draft - Connection Orchestrator

## Document Control

- Document ID: `LLD-003`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-008 Connection Orchestrator`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-001`
  - `FR-005`
  - `FR-007`
  - `SR-004`
  - `IF-001`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_orchestrator.h` | orchestration public API 정의 | Public | 상위 adapter와 action executor 경계 |
| `src/rsrx_orchestrator.c` | event 주입 및 action dispatch 구현 | Internal | 상태 머신 wrapper 역할 |
| `tests/unit/test_rsrx_orchestrator.c` | dispatch 순서 및 report 단위 테스트 | Internal | side effect callback 자체는 stub 사용 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_action_dispatch_fn` | function pointer | action 단건 dispatch callback | null 금지, 결정적이어야 함 |
| `rsrx_action_sink_t` | struct | action dispatch target | context pointer와 callback을 함께 전달 |
| `rsrx_orchestrator_context_t` | struct | state machine context와 action sink 보유 | 동적 메모리 미사용 |
| `rsrx_orchestrator_report_t` | struct | transition 결과와 dispatch 개수 보유 | 호출자 소유 버퍼 사용 |
| `rsrx_orchestrator_init` | function | orchestrator 초기화 | action sink 필수 |
| `rsrx_orchestrator_process_event` | function | event 처리 후 action dispatch | 상태 머신 결과를 변경하지 않음 |
| `rsrx_orchestrator_get_state` | function | 현재 상태 조회 | 읽기 전용 |
| `rsrx_orchestrator_reset` | function | 내부 상태 초기화 | bounded 동작 |

## Functional Behavior

- `rsrx_orchestrator_init`:
  - action sink 유효성을 검증한다.
  - 내부 `rsrx_state_machine_context_t`를 초기화한다.
- `rsrx_orchestrator_process_event`:
  - 입력 인자를 검증한다.
  - 상태 머신에 event를 전달해 `rsrx_transition_result_t`를 얻는다.
  - 반환된 action array를 배열 순서대로 `rsrx_action_dispatch_fn`에 전달한다.
  - dispatch된 action 수를 report에 기록한다.
- `rsrx_orchestrator_get_state`:
  - 상태 머신 현재 상태를 그대로 조회한다.
- `rsrx_orchestrator_reset`:
  - 상태 머신 컨텍스트를 초기화 상태로 되돌린다.

## Dispatch Policy

- dispatch 순서:
  - action은 `rsrx_transition_result_t.xActions` 배열 순서를 그대로 따른다.
  - orchestrator는 action 재정렬을 하지 않는다.
- 책임 분리:
  - orchestrator는 action 의미를 해석하지 않는다.
  - 실제 송신, 타이머 조작, 로그 기록은 action sink 구현이 담당한다.
- 오류 처리:
  - null context, null report, null action sink callback은 `RSRX_STATUS_INVALID_ARGUMENT`로 거부한다.
  - invalid argument 시 report가 제공되면 deterministic error report로 초기화한다.

## Verification Notes

- 필요한 테스트:
  - init 후 상태 조회 검증
  - event 처리 후 action dispatch 순서 검증
  - fail-safe 전이 시 dispatch 개수 검증
  - invalid argument 시 deterministic report 검증
- 분석 포인트:
  - callback null 방어
  - dispatch loop bounded 여부
  - 상태 머신 결과와 dispatch 결과의 일관성

