# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.2 (Updated to reflect Official Baseline 2026-04-30)
**Target Component:** `sil4/tests/unit/*` (단위 테스트 프레임워크)
**Focus Area:** Test Coverage & Fault Injection Scenarios (Phase 3)

본 보고서는 구현된 단위 테스트들이 정상적인 동작(Happy Path)뿐만 아니라, 열악한 철도 통신 환경에서 발생할 수 있는 결함(Faults) 및 비정상 상태를 충분히 커버(Coverage)하고 있는지 평가한 3단계 V&V 검증 결과입니다.

---

## 1. 단위 테스트 커버리지 및 결함 주입 (Fault Injection) 검증

SIL4 검증의 핵심은 **"비정상 상황(예기치 않은 패킷, 타임아웃, 링크 단절 등)에서 시스템이 예측 가능한 안전 상태(Fail-Safe)로 귀결되는가?"**를 증명하는 것입니다. 

> [!TIP]
> **V&V Finding 10: Transport Supervisor 계층의 예외 처리 테스트 (COMPLIANT / EXCELLENT)**
> * **검사 결과:** `test_rsrx_transport_supervisor.c` 분석 결과, 통신 채널의 물리적 오류 및 패킷 손상을 모사한 **Fault Injection 기법**이 매우 정교하게 적용되어 있습니다.
> * **주요 검증 시나리오:**
>   - `vTestSupervisorDecodeFailure`: 코덱 오류(패킷 훼손) 시 즉각 에러 카운터 증가
>   - `vTestSupervisorSequenceGapDetection`: 시퀀스 누락 발생 시 즉각적인 재전송(Retransmission Pending) 상태 진입 확인
>   - `vTestSupervisorStaleSequenceProtocolError`: 과거 시퀀스(Replay Attack) 수신 시 Protocol Error 로 안전하게 연결 해제(`Safe Disconnect`)
>   - `vTestSupervisorPollReceiveChannelDown` / `ErrorEscalatesAndResets`: 다중 채널 링크 단절 및 에러 예산(Error Budget) 초과 시나리오 완벽 검증

> [!TIP]
> **V&V Finding 11: Protocol Context의 시퀀스 넘버 경계값 검증 (COMPLIANT)**
> * **검사 결과:** `test_rsrx_protocol_context.c`에서 극단적인 시나리오인 **시퀀스 번호 오버플로우(Wrap Around)** 에 대한 경계값 테스트(`vTestOutboundSequenceWrapRejected`)가 꼼꼼히 작성되어 있습니다. `UINT32_MAX` 도달 시 더 이상 진행되지 않고 방어하는 로직이 성공적으로 검증되었습니다.

> [!WARNING]
> **V&V Finding 12: 상태 머신 Action Assertion 강화 (Accepted Test-Strengthening Closeout / R-007)**
> * **상황:** 이전 V&V 검토에서 `HANDSHAKE_SUCCESS` 전이 규칙의 세부 액션(Action list) 단언 밀도를 강화할 필요가 식별되었습니다.
> * **처리 상태:** `test_rsrx_state_machine.c`의 handshake-success path는 현재 baseline action ordering을 명시적으로 검증하도록 보강됐고, `RV-291` 기준으로 closeout 되었습니다.
> * **V&V 공식 의견:** 이 항목은 더 이상 열린 테스트 백로그가 아니라, 향후 상태 머신 contract 변경 시 동일 수준의 action assertion density를 유지해야 하는 회귀 방지 기준으로 관리해야 합니다.

---

## 2. 향후 통합(Integration) 및 확장성 평가

현재 단위 테스트는 Mock 객체(가짜 Codec, 가짜 Timer)를 활용해 논리적인 상태만을 잘 격리하여 테스트하고 있습니다. 

> [!NOTE]
> **V&V Finding 13: 미래 보안 요구사항(MD4 MAC, CRC)용 테스트 확장성 (ROADMAP)**
> * **상황:** 현재 베이스라인 스코프는 간소화된 스켈레톤 코덱을 기반으로 하므로, MAC/CRC에 대한 무결성 검증 테스트가 부재합니다. 
> * **권고:** 로드맵 `R-006`에 따라 향후 `rsrx_codec.c`에 실제 MAC/CRC 검증 로직이 붙을 때, 이 단위 테스트 프레임워크를 그대로 확장하여 "MAC 불일치(Spoofing)", "CRC 오류(Bit Error)" 결함 주입 테스트를 매끄럽게 추가할 수 있도록 구조가 잘 설계되어 있습니다.

---

### V&V 결론 (Phase 3)

현재 구축된 단위 테스트 환경은 **수준 높은 결함 주입(Fault Injection) 테스트와 경계값(Boundary) 테스트를 포함**하고 있습니다. 이전에 식별된 `R-007` state-machine assertion 보강 항목은 `RV-291` 기준으로 closeout 되었으며, 잔여 테스트 성숙도 과제는 codec/security parity, richer sequencing variant, runtime feedback growth 같은 future policy growth와 함께 확장되어야 합니다.
