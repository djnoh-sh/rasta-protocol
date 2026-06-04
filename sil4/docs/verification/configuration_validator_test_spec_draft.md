# Test Specification Draft - Configuration Validator

## Document Control

- Document ID: `TS-010`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-05-07`

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
| TC-CFG-003 | FR-006 | 필수 codec port 누락 검증 | `pfEncode` 또는 `pfDecode`가 null인 config 준비 | `rsrx_validate_session_config` 호출 | `MISSING_REQUIRED_FIELD`, field=`CODEC_PORT` 반환 | outbound encode 및 inbound decode 경계 누락이 명확히 식별된다 |
| TC-CFG-004 | FR-006 | interval range 오류 검증 | supervision interval이 0인 config 준비 | `rsrx_validate_session_config` 호출 | `INVALID_RANGE`, field=`SUPERVISION_INTERVAL` 반환 | 0 interval 설정으로 startup을 허용하지 않는다 |
| TC-CFG-005 | FR-006, SR-003 | payload 일관성 오류 검증 | payload pointer는 null이고 length는 0보다 큰 config 준비 | `rsrx_validate_session_config` 호출 | `INCONSISTENT_VALUE`, field=`FRAME_PAYLOAD` 반환 | 상충된 설정 조합이 결정적으로 거부된다 |
| TC-CFG-006 | SR-003 | invalid argument/report guard 검증 | null config, null report, stale validation report | `rsrx_validate_session_config` 호출 | null config는 `INVALID_ARGUMENT`, valid config는 stale report를 `OK/NONE`으로 clear, null report가 있는 invalid helper path도 status를 직접 반환 | UB 없이 명시적 오류를 반환하고 stale validation report 또는 null-report 역참조를 남기지 않는다 |
| TC-CFG-007 | FR-006, SR-003, IF-002 | default channel topology 일관성 검증 | 기본 channel이 channel manager declared channel list에 없는 config 준비 | `rsrx_validate_session_config` 호출 | `INCONSISTENT_VALUE`, field=`DEFAULT_CHANNEL` 반환 | default outbound channel과 redundancy topology가 불일치하는 설정은 startup에서 결정적으로 거부된다 |
| TC-CFG-008 | FR-006, SR-003, IF-002 | duplicate channel priority topology 거부 검증 | active-standby channel manager config의 두 channel priority가 같은 session config 준비 | `rsrx_validate_session_config` 호출 | `INVALID_RANGE`, field=`DEFAULT_CHANNEL` 반환 | ambiguous redundancy topology가 session startup gate에서도 결정적으로 거부된다 |
| TC-CFG-009 | FR-006, SR-001, SR-003, IF-002 | CRC-required codec policy startup gate 검증 | `uRequireCrc=1`인 session config와 default codec port 또는 CRC32 codec port 준비 | `rsrx_validate_session_config` 호출 | default codec port는 `INCONSISTENT_VALUE`, field=`CODEC_PORT`로 거부되고 CRC32 codec port는 `OK` | deployment policy가 CRC를 요구할 때 startup이 non-CRC codec 선택을 허용하지 않고 explicit CRC32 codec 선택만 통과시킨다 |
| TC-CFG-010 | FR-006, SR-001, SR-003, IF-002 | unavailable MAC/timestamp policy startup gate 검증 | `uRequireMac=1` 또는 `uRequireTimestamp=1`인 session config 준비 | `rsrx_validate_session_config` 호출 | `INCONSISTENT_VALUE`, field=`CODEC_PORT` 반환 | current codec capability가 제공하지 않는 MAC/timestamp security policy를 startup에서 silent downgrade 없이 거부한다 |
| TC-CFG-011 | FR-006, SR-003, IF-002 | platform critical-section port startup gate 검증 | `pfEnter` 또는 `pfExit`가 null인 critical-section port를 가진 session config 준비 | `rsrx_validate_session_config` 호출 | `MISSING_REQUIRED_FIELD`, field=`PLATFORM_CRITICAL_SECTION` 반환 | public API concurrency policy 적용 전에 locking port 누락을 startup에서 결정적으로 거부한다 |
