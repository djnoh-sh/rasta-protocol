# System Requirements Draft

## Document Control

- Document ID: `SRS-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Reviewers: `TBD`
- Last Updated: `2026-03-13`

## Scope

- 대상 기능:
  - 안전 관련 RaSTA 통신 스택의 재구현
  - redundancy layer, SR layer, configuration, diagnostics, public API의 재정의
  - 향후 SCI 계층 상위 적용을 고려한 하부 통신 기반 제공
- 제외 범위:
  - 기존 프로토타입 코드의 직접 인증
  - 임시 예제 프로그램의 인증
  - UI, 운영 도구, 배포 자동화 세부사항
- 관련 상위 시스템:
  - 철도/신호 안전 통신이 필요한 상위 제어 시스템

## References

- 관련 표준:
  - `EN 50126`
  - `EN 50128`
  - `EN 50129`
  - `MISRA-C:2012`
- 관련 외부 명세:
  - `DIN VDE V 0831-200`
- 관련 내부 문서:
  - [SIL4_REIMPLEMENTATION_RULES.md](/home/djnoh/repos/rasta-protocol/SIL4_REIMPLEMENTATION_RULES.md)
  - [codebase_detailed_analysis.md](/home/djnoh/repos/rasta-protocol/md_doc/codebase_detailed_analysis.md)

## Requirement List

| Req ID | Type | Title | Description | Rationale | Verification Method | Safety Impact | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FR-001 | Functional | Deterministic Startup | 시스템은 명시적 초기화 API 호출 이후에만 통신 기능을 활성화해야 한다. 초기화 완료 전 송수신 API 호출은 정의된 오류로 거부되어야 한다. | 초기화 이전 비정상 상태 진입 방지 | Test / Review | High | Draft |
| FR-002 | Functional | Controlled Connection Establishment | 시스템은 정의된 핸드셰이크 절차를 통해서만 통신 연결을 성립해야 하며, 허용되지 않은 순서의 메시지는 연결 종료 또는 안전 처리로 이어져야 한다. | 비정상 연결 상태 방지 | Test / Review | High | Draft |
| FR-003 | Functional | Ordered Data Delivery | 시스템은 유효한 수신 데이터만 상위 계층에 전달해야 하며, 순서 보장이 필요한 경우 정의된 순서대로 전달해야 한다. | 데이터 무결성 확보 | Test / Analysis | High | Draft |
| FR-004 | Functional | Retransmission Handling | 시퀀스 누락 또는 확인 불일치가 발생하면 정의된 retransmission 절차를 수행해야 한다. | 손실/지연 상황 대응 | Test / Review | High | Draft |
| FR-005 | Functional | Controlled Disconnect | 사용자 요청, timeout, 프로토콜 오류, 버전 불일치 상황에서 시스템은 정의된 종료 절차를 수행해야 한다. | fail-safe 종료 보장 | Test / Review | High | Draft |
| FR-006 | Functional | Configuration Validation | 시스템은 시작 시 모든 필수 설정값의 타입, 범위, 일관성을 검증해야 하며 오류 설정으로 동작을 시작해서는 안 된다. | 잘못된 설정으로 인한 위험 방지 | Test / Review | High | Draft |
| FR-007 | Functional | Diagnostic Reporting | 시스템은 진단 및 오류 정보를 구조적으로 수집하고 외부 검토가 가능한 형태로 제공해야 한다. | 운영 및 인증 증빙 지원 | Test / Review | Medium | Draft |
| SR-001 | Safety | Fail-Safe on Invalid Message | 허용되지 않은 메시지 타입, 인증 실패, 시퀀스 오류, 구조 오류가 발생하면 시스템은 정의된 안전 상태로 전이해야 한다. | 위험 상태 지속 방지 | Test / Analysis | High | Draft |
| SR-002 | Safety | Timeout Supervision | heartbeat 또는 동등한 감독 메커니즘이 정의된 시간 내에 만족되지 않으면 시스템은 연결을 안전 종료해야 한다. | 상대 노드 장애 검출 | Test / Analysis | High | Draft |
| SR-003 | Safety | No Undefined Runtime Behavior | 인증 대상 구현은 초기화되지 않은 값 사용, 잘못된 메모리 접근, 정의되지 않은 상태 전이와 같은 비결정 동작을 허용해서는 안 된다. | SIL4 수준의 예측 가능성 확보 | Review / Analysis / Test | High | Draft |
| SR-004 | Safety | Traceable Error Handling | 모든 오류 처리 경로는 요구사항, 설계, 로그, 테스트 항목과 추적 가능해야 한다. | 인증 심사 대응 | Review / Analysis | Medium | Draft |
| IF-001 | Interface | Explicit Public API Contract | 공개 API는 입력, 출력, 오류 반환, 호출 순서, 상태 제약을 문서화해야 한다. | 오용 방지 | Review | Medium | Draft |
| IF-002 | Interface | Platform Isolation | 플랫폼 의존 기능은 추상화 계층 뒤로 격리되어야 하며, 상위 로직은 OS API에 직접 의존해서는 안 된다. | 이식성 및 검증성 향상 | Review / Test | Medium | Draft |

## Interfaces

- 입력:
  - 초기화 요청
  - 연결 요청
  - 송신 요청
  - 수신된 transport packet
  - timeout / timer event
  - 설정 파일 또는 설정 구조체
- 출력:
  - 전송 packet
  - 수신 application data
  - 상태 변화 통지
  - 진단/오류 로그
- 오류 입력:
  - 잘못된 packet 구조
  - 잘못된 sequence / timestamp
  - timeout
  - 설정 오류
  - 자원 부족
- 시간 제약:
  - startup validation은 통신 시작 전에 완료되어야 한다
  - timeout 감시는 monotonic clock 기반으로 동작해야 한다
  - 재전송 및 disconnect 전이는 요구된 상한 시간을 가져야 한다

## Acceptance Criteria

- 요구사항 ID별 검증 항목이 정의되어야 한다.
- 상태 전이와 오류 처리 경로가 설계 문서에 명시되어야 한다.
- 요구사항-설계-코드-테스트 추적 매트릭스가 시작되어야 한다.

## Open Issues

- OI-001: heartbeat 관련 시간 상수와 허용 jitter 범위를 어떤 수준으로 고정할지 결정 필요
- OI-002: redundancy 채널 수와 failover 정책을 상위 시스템 요구사항과 연계해 확정할 필요
- OI-003: 공개 API를 C 단일 API로 유지할지, safety kernel과 adapter를 분리할지 결정 필요

