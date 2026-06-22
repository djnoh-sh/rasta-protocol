# Hazard Log Draft

## Document Control

- Document ID: `HZLOG-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-13`

## Hazard Entries

| Hazard ID | Description | Cause | Effect | Severity | Mitigation | Verification | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| HZ-001 | 잘못된 메시지 수용 | 메시지 타입 검증 실패, 버전 검증 실패, 인증 실패 | 잘못된 연결 성립 또는 비정상 데이터 처리 | High | `SR-001`, 상태 머신 명세, 인터페이스 검증, parser 검증 | Test / Review / Analysis | Open |
| HZ-002 | timeout 미검출 또는 오검출 | 타이머 설계 오류, monotonic clock 미사용, 상태 불일치 | 장애 상대를 감지하지 못하거나 정상 연결을 잘못 종료 | High | `SR-002`, 타이밍 요구사항 명시, timeout 테스트, 경계값 시험 | Test / Analysis | Open |
| HZ-003 | 시퀀스 불일치 처리 실패 | retransmission 상태 설계 미흡, 확인 번호 처리 오류 | 데이터 순서 깨짐, 중복 전달, 손실 은폐 | High | `FR-003`, `FR-004`, 상태 전이 검증, known-answer 테스트 | Test / Review | Open |
| HZ-004 | 설정 오류 상태로 시스템 기동 | 설정값 범위/타입 검증 누락 | 위험한 파라미터로 런타임 동작 | High | `FR-006`, startup validation, invalid-config 테스트 | Test / Review | Open |
| HZ-005 | 정의되지 않은 런타임 동작 | 초기화되지 않은 변수, 잘못된 포인터, 메모리 오용 | 예측 불가 동작, 안전 기능 상실 | High | `SR-003`, MISRA 적용, 정적분석, 경고 0건 정책 | Analysis / Review / Test | Open |
| HZ-006 | 오류 처리의 비추적성 | 오류 코드 무시, 로그 누락, 문서 미반영 | 결함 원인 분석 실패, 인증 증빙 부족 | Medium | `SR-004`, 오류 처리 표준화, traceability matrix 유지 | Review / Analysis | Open |
| HZ-007 | 플랫폼 의존성 노출 | 상위 로직이 직접 OS API 사용 | 이식성 저하, 검증 환경 차이 | Medium | `IF-002`, platform abstraction 계층 분리 | Review / Test | Open |

## Assumptions

- 상위 시스템은 통신 실패 시 안전한 상위 동작 정책을 가진다.
- 적용 대상 플랫폼은 monotonic clock과 결정적 timer 인터페이스를 제공한다.
- redundancy 채널 수와 배치 방식은 상위 시스템 설계 단계에서 확정된다.

## Residual Risks

- RR-001: 적용 표준의 해석에 따라 추가 hazard 분해가 필요할 수 있다.
- RR-002: 실제 target platform 제약이 확인되면 timeout 및 자원 관련 hazard가 더 세분화될 수 있다.

