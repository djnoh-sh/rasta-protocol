# Low-Level Design Draft - Platform Adapter Layer

## Document Control

- Document ID: `LLD-005`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-007 Platform Abstraction`
  - `MOD-008 Connection Orchestrator`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `IF-002`
  - `FR-007`
  - `SR-003`
  - `SR-004`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_platform_adapters.h` | platform-backed executor helper 공개 API | Public | timer/diagnostics executor binding |
| `src/rsrx_platform_adapters.c` | platform-backed executor 구현 | Internal | orchestrator executor table 조립 |
| `tests/unit/test_rsrx_platform_adapters.c` | adapter binding 및 dispatch 단위 테스트 | Internal | platform port stub 사용 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_platform_adapter_context_t` | struct | platform port와 interval 설정 보유 | 동적 메모리 미사용 |
| `rsrx_platform_adapter_init` | function | platform adapter context 초기화 | 유효한 port table 필요 |
| `rsrx_platform_timer_executor_dispatch` | function | timer action을 platform timer command로 변환 | timer action만 처리 |
| `rsrx_platform_diagnostics_executor_dispatch` | function | transition result를 diagnostics record로 변환 | bounded 기록 생성 |
| `rsrx_platform_adapter_build_executor_table` | function | transport/api/lifecycle executor와 platform-backed executor를 결합 | null 금지 |

## Functional Behavior

- timer adapter:
  - `START_SUPERVISION_TIMER`, `RESET_SUPERVISION_TIMER`를 monotonic deadline 기반 command로 변환한다.
  - 현재 시간은 `rsrx_clock_port_t`를 통해 조회한다.
- diagnostics adapter:
  - transition result를 `rsrx_diagnostic_record_t`로 변환한다.
  - severity는 `diagnostic code`에서 결정한다.
  - event counter는 adapter context 내부에서 증가시킨다.
- executor table builder:
  - transport/api/lifecycle executor는 외부 제공 executor를 사용한다.
  - timer/diagnostics executor는 platform adapter dispatch 함수로 설정한다.

## Verification Notes

- 필요한 테스트:
  - executor table 조립 검증
  - timer action -> timer command 변환 검증
  - diagnostics action -> diagnostic record 변환 검증
- 분석 포인트:
  - monotonic deadline 계산 bounded 여부
  - severity mapping 완전성
  - platform callback 실패가 상위 상태 머신 결정성에 영향을 주지 않도록 유지
