# Low-Level Design Draft - Platform Abstraction

## Document Control

- Document ID: `LLD-004`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 모듈:
  - `MOD-007 Platform Abstraction`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `IF-002`
  - `SR-003`
  - `SR-004`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_platform.h` | platform abstraction public contract | Public | clock, timer, diagnostics, critical-section port 정의 |
| `tests/unit/test_rsrx_platform_contract.c` | header contract smoke test | Internal | 타입 및 인터페이스 계약 확인 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_monotonic_time_ns_t` | typedef | monotonic time 표현 | unsigned 64-bit 고정 |
| `rsrx_platform_status_t` | enum | platform layer 결과 코드 | 플랫폼 오류를 상위 정책과 분리 |
| `rsrx_timer_id_t` | enum | timer 식별자 | 명시된 timer만 사용 |
| `rsrx_timer_command_type_t` | enum | timer 제어 동작 | start/restart/cancel만 허용 |
| `rsrx_log_severity_t` | enum | 진단 severity | 운영 로그 severity 분류 |
| `rsrx_timer_command_t` | struct | timer executor 입력 계약 | deadline와 reason 포함 |
| `rsrx_diagnostic_record_t` | struct | diagnostics executor 입력 계약 | state/status/reason/diagnostic 포함 |
| `rsrx_clock_port_t` | struct | monotonic clock 포트 | `pfNow` 필수 |
| `rsrx_timer_port_t` | struct | timer command 포트 | `pfCommand` 필수 |
| `rsrx_diagnostics_port_t` | struct | diagnostics sink 포트 | `pfWrite` 필수 |
| `rsrx_critical_section_port_t` | struct | shared session-state 보호를 위한 critical-section 포트 | `pfEnter`, `pfExit` 필수 |
| `rsrx_platform_port_table_t` | struct | platform service 집합 | 모든 포트는 초기화 시 명시적으로 채움 |

## Functional Behavior

- clock 포트:
  - 상태 머신 외부 모듈은 wall-clock이 아닌 monotonic clock만 사용한다.
  - 시간 획득 실패는 platform status로 반환한다.
- timer 포트:
  - supervision/retransmission/diagnostic flush timer를 명시적 ID로 제어한다.
  - 상위 모듈은 deadline 값을 절대 monotonic time 기준으로 전달한다.
- diagnostics 포트:
  - 상태 전이 결과를 구조화된 record로 기록한다.
  - free-form 문자열 포맷팅은 platform layer 바깥의 책임이 아니다.
- critical-section 포트:
  - public API와 background/event path가 공유하는 session state 보호를 위한 enter/exit seam을 제공한다.
  - portable core는 OS API를 직접 호출하지 않으며, SafeRTOS binding은 target adapter에서 제공한다.

## Design Rules

- 상위 모듈은 직접 OS API를 호출하지 않는다.
- timer와 diagnostics executor는 `rsrx_platform.h` 타입만 사용한다.
- public API locking policy는 `rsrx_critical_section_port_t`를 통해서만 platform synchronization primitive에 접근한다.
- platform port table은 초기화 시점에만 설정하고, 런타임 중 교체하지 않는다.
- 모든 platform callback은 bounded 시간 내 복귀해야 한다.

## Verification Notes

- 필요한 테스트:
  - platform header compile contract 검증
  - timer command 구조체 필드 계약 검증
  - diagnostic record 구조체 필드 계약 검증
  - critical-section port 구조체 필드 계약 검증
- 분석 포인트:
  - wall-clock 타입 혼입 금지
  - unsigned overflow 위험 검토
  - 플랫폼 오류 코드와 상위 상태 코드 혼동 금지
