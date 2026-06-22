# Integration Tests

통합 테스트는 모듈 간 인터페이스와 상태 전이, 타이머, 오류 전파를 검증하기 위한 위치다.

규칙:

- 인터페이스 계약과 상태 머신 검증을 포함한다.
- 단위 테스트로 충분하지 않은 상호작용을 다룬다.
- 가능한 한 real codec과 fake transport/platform 조합을 우선 사용한다.
- 첫 harness는 `session + supervisor + pump_receive + outbound send` happy path를 검증한다.
