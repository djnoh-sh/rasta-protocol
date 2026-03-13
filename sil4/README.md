# SIL4 Reimplementation Workspace

## Purpose

이 디렉터리는 기존 프로토타입 구현과 분리된 SIL4 재구현 작업 공간이다.
인증 대상 산출물은 여기에서 시작하고, 기존 `src/`, `examples/`, `tests/`
코드는 참고 자료로만 사용한다.

## Top-Level Structure

- `docs/requirements`: 기능 요구사항, 안전 요구사항, 인터페이스 요구사항
- `docs/design/hld`: 상위 설계 문서
- `docs/design/lld`: 상세 설계 문서
- `docs/verification`: 테스트 명세, 검증 기록, 커버리지, 정적분석 결과
- `docs/safety`: hazard log, safety case 입력 자료, FMEA/FTA 초안
- `docs/traceability`: 요구사항-설계-코드-테스트 추적 자료
- `docs/reviews`: 설계 리뷰, 코드 리뷰, 독립 검토 기록
- `docs/templates`: 산출물 작성 템플릿
- `src`: 인증 대상 구현 코드
- `include`: 인증 대상 공개 헤더
- `platform`: 플랫폼 의존 계층
- `tests/unit`: 단위 테스트
- `tests/integration`: 통합 테스트
- `tests/system`: 시스템/시나리오 테스트
- `tools`: 빌드, 정적분석, 추적성, 문서 생성 지원 스크립트

## Working Rules

- 새 구현은 `SIL4_REIMPLEMENTATION_RULES.md`를 따른다.
- 프로토타입 코드 복사는 금지한다.
- 새 코드 추가 전 관련 요구사항/설계 문서를 먼저 만든다.
- 각 변경은 요구사항 ID와 테스트 ID를 가져야 한다.

## Suggested Initial Milestones

1. `docs/requirements`에 시스템 요구사항 초안 작성
2. `docs/safety`에 hazard log 초안 작성
3. `docs/design/hld`에 아키텍처 초안 작성
4. `docs/traceability`에 추적성 매트릭스 시작
5. 이후 `src`, `include`, `tests`에 점진 구현

