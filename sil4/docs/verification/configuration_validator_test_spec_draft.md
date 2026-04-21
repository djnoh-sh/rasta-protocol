# Test Specification Draft - Configuration Validator

## Document Control

- Document ID: `TS-010`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Scope

- 테스트 대상:
  - `MOD-005 Configuration Validator`
- 관련 요구사항:
  - `FR-006`
  - `SR-003`
- 테스트 레벨: `Unit`

## Test Cases

| Test ID | Req ID | Objective | Precondition | Stimulus | Expected Result | Pass/Fail Criteria |
| --- | --- | --- | --- | --- | --- | --- |
| TC-CFG-001 | FR-006 | valid configuration 허용 검증 | 필수 port, callback, interval이 채워진 session config 준비 | `rsrx_validate_session_config` 호출 | `OK` 반환, field=`NONE` | 결정적 허용 결과가 반환된다 |
| TC-CFG-002 | FR-006 | 필수 transport port 누락 검증 | `pfSend`가 null인 config 준비 | `rsrx_validate_session_config` 호출 | `MISSING_REQUIRED_FIELD`, field=`TRANSPORT_PORT` 반환 | 누락 필드가 명확히 식별된다 |
| TC-CFG-003 | FR-006 | 필수 codec port 누락 검증 | `pfEncode`가 null인 config 준비 | `rsrx_validate_session_config` 호출 | `MISSING_REQUIRED_FIELD`, field=`CODEC_PORT` 반환 | outbound encode 경계 누락이 명확히 식별된다 |
| TC-CFG-004 | FR-006 | interval range 오류 검증 | supervision interval이 0인 config 준비 | `rsrx_validate_session_config` 호출 | `INVALID_RANGE`, field=`SUPERVISION_INTERVAL` 반환 | 0 interval 설정으로 startup을 허용하지 않는다 |
| TC-CFG-005 | FR-006, SR-003 | payload 일관성 오류 검증 | payload pointer는 null이고 length는 0보다 큰 config 준비 | `rsrx_validate_session_config` 호출 | `INCONSISTENT_VALUE`, field=`FRAME_PAYLOAD` 반환 | 상충된 설정 조합이 결정적으로 거부된다 |
| TC-CFG-006 | SR-003 | invalid argument 검증 | null config | `rsrx_validate_session_config` 호출 | `INVALID_ARGUMENT` 반환 | UB 없이 명시적 오류 반환 |
| TC-CFG-007 | FR-006, SR-003, IF-002 | default channel topology 일관성 검증 | 기본 channel이 channel manager declared channel list에 없는 config 준비 | `rsrx_validate_session_config` 호출 | `INCONSISTENT_VALUE`, field=`DEFAULT_CHANNEL` 반환 | default outbound channel과 redundancy topology가 불일치하는 설정은 startup에서 결정적으로 거부된다 |
