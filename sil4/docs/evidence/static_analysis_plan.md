# Static Analysis And MISRA Evidence Plan

## Document Control

- Document ID: `EVID-001`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-03-17`

## Purpose

이 문서는 `sil4/` 재구현 코드베이스에 대해 정적분석과 MISRA 증빙을 어떤 방식으로 수집하고 유지할지 정의한다.

목표는 다음과 같다.

1. 새 코드에 대한 경고와 규칙 위반을 작업 종료 전에 검출한다.
2. MISRA 편차를 임시 메모가 아니라 통제된 기록으로 관리한다.
3. 심사 대응 시 재현 가능한 분석 절차와 결과 패키지를 제공한다.

## Scope

- 대상 코드:
  - `sil4/include`
  - `sil4/src`
  - `sil4/tests/unit`
  - `sil4/tests/integration`
- 제외 대상:
  - 기존 프로토타입 코드
  - 외부 툴체인 내부 헤더
  - 생성 산출물과 임시 빌드 디렉터리

## Evidence Set

| Evidence ID | Artifact | Purpose | Owner | Update Trigger |
| --- | --- | --- | --- | --- |
| EVID-SA-001 | `static_analysis_plan.md` | 분석 범위와 절차 기준 | Project Team | 정책 변경 시 |
| EVID-SA-002 | `static_analysis_checklist.md` | 실행 전/후 확인 항목 | Implementer | 분석 실행 시 |
| EVID-SA-003 | `misra_deviation_log.md` | 편차와 승인 상태 관리 | Project Team | 편차 발생 시 |
| EVID-SA-004 | `templates/static_analysis_report_template.md` | 결과 보고서 형식 통일 | Project Team | 템플릿 개선 시 |
| EVID-SA-005 | `reports/` 하위 분석 결과 | 실행 결과 보관 | Implementer | 분석 실행 시 |

## Analysis Gates

### Gate SA-1

- 시점: 새 모듈 최초 추가 시
- 최소 요구:
  - 컴파일 경고 0건
  - 고위험 MISRA 위반 0건 또는 승인된 편차만 존재
  - 결과 요약이 보고서에 기록됨

### Gate SA-2

- 시점: 인터페이스 변경 또는 안전 관련 로직 변경 시
- 최소 요구:
  - 변경 파일 전수 분석
  - 신규 경고 0건
  - 신규 편차는 `misra_deviation_log.md`에 등록됨
  - 리뷰 기록에 분석 결과 참조가 포함됨

### Gate SA-3

- 시점: 통합 검증 묶음 종료 시
- 최소 요구:
  - `sil4/src`와 `sil4/include` 전체 분석
  - open 편차 목록과 mitigation 상태 정리
  - 재현 절차와 툴 버전 기록

## Execution Policy

- 기본 원칙:
  - 분석은 로컬 개발 편의 기능이 아니라 DoD 항목이다.
  - 신규 경고는 허용하지 않는다.
  - 편차는 “나중에 수정” 대신 등록, 영향도 평가, 승인 상태를 남긴다.
- 기록 원칙:
  - 분석 일시, 커밋 ID, 대상 경로, 툴 버전, 옵션을 기록한다.
  - 결과는 요약과 상세 로그를 분리한다.
  - 보고서에는 “총 경고 수”보다 “신규/잔존/승인 편차”를 우선 기록한다.

## Tooling Baseline

현재 baseline은 다음과 같이 확정한다.

1. compiler warning gate (`gcc`, `-Wall -Wextra -Werror`)
2. `cppcheck` baseline scan
3. MISRA-oriented checklist review

세부 tool/version/profile은 `tooling/static_analysis_toolchain_baseline.md`를 기준으로 관리한다.

후속 단계에서 아래를 추가 확정한다.

1. CI 실행 방식
2. fail threshold
3. 전용 MISRA analyzer 도입 여부

## Required Report Content

각 분석 보고서는 최소 아래 항목을 포함해야 한다.

- Report ID
- 대상 커밋 ID
- 분석 대상 파일 목록
- 툴/버전/실행 옵션
- 결과 요약:
  - 신규 경고 수
  - 잔존 경고 수
  - 신규 편차 수
  - open 편차 수
- 고위험 결과 상세
- 조치 계획
- 관련 Review ID

## Entry Criteria

- 분석 대상 코드가 빌드 가능해야 한다.
- 요구사항/설계 문서 참조가 존재해야 한다.
- 변경 파일 목록이 확정돼야 한다.

## Exit Criteria

- 분석 체크리스트가 완료됐다.
- 신규 고위험 이슈가 없다.
- 편차 발생 시 편차 로그가 갱신됐다.
- 리뷰 기록에 분석 결과 참조가 남았다.

## Open Items

| OI ID | Description | Owner | Target |
| --- | --- | --- | --- |
| OI-SA-001 | MISRA rule subset과 severity mapping 정의 | Project Team | P5 진행 중 |
| OI-SA-002 | CI에서의 자동 실행 정책 확정 | Project Team | SA-2 이전 |
| OI-SA-003 | `cppcheck` baseline finding cleanup 및 follow-up report 생성 | Project Team | SA-2 이전 |
