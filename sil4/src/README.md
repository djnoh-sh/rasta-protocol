# SIL4 Source Directory

이 디렉터리는 인증 대상 제품 코드를 위한 위치다.

규칙:

- 요구사항과 설계 없이 새 파일을 추가하지 않는다.
- public API는 `../include`에, internal 구현은 이 디렉터리에 둔다.
- 플랫폼 의존 코드는 가능하면 `../platform` 아래로 격리한다.

