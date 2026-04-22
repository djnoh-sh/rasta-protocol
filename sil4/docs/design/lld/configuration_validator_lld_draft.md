# Low-Level Design Draft - Configuration Validator

## Document Control

- Document ID: `LLD-010`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-04-22`

## Scope

- 대상 모듈:
  - `MOD-005 Configuration Validator`
- 관련 HLD:
  - `HLD-001`
- 관련 요구사항:
  - `FR-006`
  - `SR-003`

## File Structure

| File | Purpose | Public/Internal | Notes |
| --- | --- | --- | --- |
| `include/rsrx_config_validator.h` | 설정 검증 계약과 보고 타입 정의 | Public | startup gate contract |
| `src/rsrx_config_validator.c` | 설정값 필수 필드/범위/일관성 검증 구현 | Internal | deterministic validation only |
| `tests/unit/test_rsrx_config_validator.c` | validator 단위 테스트 | Internal | valid/invalid config 조합 검증 |

## Types and Interfaces

| Element | Kind | Description | Constraints |
| --- | --- | --- | --- |
| `rsrx_config_status_t` | enum | 설정 검증 결과 분류 | `OK`, `INVALID_ARGUMENT`, `MISSING_REQUIRED_FIELD`, `INVALID_RANGE`, `INCONSISTENT_VALUE` |
| `rsrx_config_field_t` | enum | 오류가 발생한 설정 필드 식별 | caller가 원인 필드를 추적할 수 있어야 함 |
| `rsrx_config_validation_report_t` | struct | 검증 결과와 오류 필드를 함께 반환 | 동적 메모리 없음 |
| `rsrx_validate_session_config` | function | session config의 필수값, 범위, 일관성 검증 | side effect 없이 결정적 결과 반환 |

## Functional Behavior

- `rsrx_validate_session_config`:
  - null config를 즉시 거부한다.
  - transport `send/receive/query` 포트가 모두 정의됐는지 확인한다.
  - codec `encode` 포트가 정의됐는지 확인한다.
  - platform `clock/timer/diagnostics` 포트가 모두 정의됐는지 확인한다.
  - API callback과 lifecycle callback이 모두 정의됐는지 확인한다.
  - 기본 channel이 `INVALID`인지 검사한다.
  - channel manager startup validation을 재사용해 ambiguous redundancy topology를 거부한다.
  - 기본 channel이 channel manager topology의 declared channel list에 포함되는지 검사한다.
  - payload pointer와 payload length가 일관적인지 검사한다.
  - supervision/retransmission/diagnostic flush interval이 0이 아닌지 검사한다.
  - 첫 번째 오류를 보고서에 기록하고 즉시 반환한다.

## Verification Notes

- 필요한 테스트:
  - valid config 허용 검증
  - 필수 transport port 누락 검증
  - 필수 codec port 누락 검증
  - interval range 오류 검증
  - payload 일관성 오류 검증
  - 기본 channel / channel manager topology 일관성 검증
  - duplicate channel priority topology 거부 검증
  - invalid argument 검증
- 분석 포인트:
  - 검증 함수는 side effect가 없어야 한다.
  - 오류 분류와 오류 필드는 첫 실패 지점에서 결정적으로 고정되어야 한다.
