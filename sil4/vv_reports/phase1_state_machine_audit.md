# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.2 (Updated to reflect Official Baseline 2026-04-30)
**Target Component:** `rsrx_state_machine.c`, `rsrx_protocol_context.c`
**Focus Area:** State Machine Transitions & Protocol Context (Phase 1)

본 V&V 보고서는 작성된 SIL4 대상 코드(C 언어)가 현재 프로젝트에서 합의된 베이스라인(Baseline Contract) 및 RaSTA 표준(DIN VDE V 0831-200 / EN 50159) 규격을 얼마나 충족하는지 검증한 결과입니다.

---

## 1. 상태 머신 전이 규칙 검증 (State Machine Transition Audit)

현재의 `xTransitionRules` 구현은 프로젝트의 현행 베이스라인 의도를 충실히 따르고 있으나, 엄격한 RaSTA 표준(5.5장)과의 비교 시 호환성 확보를 위해 향후 해석 및 반영이 필요한 갭(Parity Gap)이 존재합니다.

> [!WARNING] 
> **V&V Finding 1: 3-way Handshake 과정의 호환성 차이 (Parity Interpretation Candidate)**
> * **규격 요구사항:** 초기화자(Client)가 `ConnResp` 메시지를 수신하여 `Start`(현재 `CONNECTING`) 상태에서 `Up`(현재 `ESTABLISHED`) 상태로 전이할 때, `Heartbeat` 또는 `Data` 메시지를 송신해야 합니다.
> * **현재 구현 상태:** 베이스라인 코드에는 `RSRX_ACTION_SEND_HEARTBEAT`가 누락되어 있습니다.
> * **V&V 공식 의견:** 이는 현재 합의된 베이스라인 상의 결함(Defect)은 아니나, 향후 타 규격 장비와의 완벽한 상호 운용성(Interoperability) 및 SIL4 인증을 위해서는 명세가 갱신되어야 할 미래 호환성(Protocol Parity) 후보입니다.

> [!WARNING]
> **V&V Finding 2: Server의 Start 상태에서 Heartbeat 수신 처리 (Parity Interpretation Candidate)**
> * **상황:** Server가 `Start`(`CONNECTING`) 상태에서 `Heartbeat` 수신 시 `ESTABLISHED`로 전이되지 않고 유지되는 구조입니다.
> * **V&V 공식 의견:** Finding 1과 마찬가지로 현재 베이스라인의 의도된 설계이나, 향후 규격 호환성 달성을 위해 재해석이 필요한 스펙 차이입니다.

---

## 2. 프로토콜 컨텍스트 및 시퀀스 검증 (Protocol Context Audit)

> [!IMPORTANT]
> **V&V Finding 3: 초기 `CONNECT_REQUEST` 시퀀싱 해석 (Baseline Decision Closed / Future Parity Growth)**
> * **상황:** 현 베이스라인은 `CONNECT_REQUEST`를 시퀀스 메시지가 아니라 연결 진입을 위한 unsequenced admission trigger로 명시적으로 고정했습니다.
> * **현재 처리 상태:** 이 결정은 `TC-PC-026` 및 `RV-292` 기준으로 검증/리뷰 closeout 되었으며, 현재 베이스라인의 미해결 결함으로 분류하지 않습니다.
> * **V&V 공식 의견:** 향후 더 엄격한 RaSTA parity가 필요하면 `R-001`의 next protocol sequencing growth 항목에서 `CONNECT_REQUEST`를 sequenced family로 재정의할지 별도 정책 결정이 필요합니다.

> [!NOTE]
> **V&V Finding 4: Heartbeat 발송 주기(Th) 타이머 부재 (ENHANCEMENT)**
> * 수신 감시용 `Supervision Timer`만 존재하며, 능동적 발송을 위한 타이머 이벤트 구조가 누락되어 있습니다.

---

### V&V 결론 (Phase 1)

현재 상태 머신(`rsrx_state_machine.c`)은 **프로젝트의 관리적 베이스라인을 충실히 반영**하고 있으므로 당장의 오류(Defect)로 분류되지 않습니다. 다만 V&V 관점에서는 식별된 **상태 전이 규격 불일치 후보(Finding 1, 2)** 및 `CONNECT_REQUEST`의 미래 parity 재검토 가능성(Finding 3)이 향후 최종 SIL4 규격 획득 범위에서 인증 리스크(Certification Risk)로 작용할 수 있음을 경고하며, 로드맵의 future growth 항목으로 계속 관리되어야 합니다.
