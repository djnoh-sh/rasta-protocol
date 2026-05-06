# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.4 (V&V Independent Re-assessment - Objective)
**Target Component:** `rsrx_state_machine.c`, `rsrx_protocol_context.c`
**Focus Area:** State Machine Transitions & Protocol Context (Phase 1)

본 V&V 보고서는 대상 코드가 RaSTA 표준(DIN VDE V 0831-200 / EN 50159)의 요구사항을 충족하는지 객관적 사실(Fact)을 바탕으로 검증한 결과입니다.

---

## 1. 상태 머신 전이 규칙 (State Machine Transition Audit)

> [!WARNING] 
> **V&V Finding 1: 3-way Handshake 연결 성립 절차 규격 호환성 개선 권고**
> * **기술적 사실:** RaSTA 규격 5.5.2절에 따르면, 초기화자(Client)가 `ConnResp` 메시지를 수신하여 `Start` 상태에서 `Up` 상태로 전이할 때 `Heartbeat` 또는 `Data` 메시지를 송신하여 응답해야 합니다. 현재 구현(`RSRX_EVENT_HANDSHAKE_SUCCESS`)에는 해당 송신 액션이 누락되어 있습니다.
> * **개선 권고:** 타 장비와의 상호 운용성(Interoperability) 심사 시 타임아웃 발생 위험이 있으므로, 전이 규칙에 `RSRX_ACTION_SEND_HEARTBEAT`를 추가할 것을 권고합니다.

> [!WARNING]
> **V&V Finding 2: Server의 Start 상태 내 Heartbeat 수신 처리 개선 권고**
> * **기술적 사실:** Server가 `Start`(`CONNECTING`) 상태에서 유효한 `Heartbeat`를 수신했을 때 `ESTABLISHED`로 상태가 전이되지 않고 `CONNECTING` 상태를 유지합니다.
> * **개선 권고:** 표준 전이 모델과 일치시키기 위해 해당 이벤트 발생 시 상태가 `ESTABLISHED`로 전이되도록 로직 개선을 권고합니다.

---

## 2. 프로토콜 컨텍스트 및 시퀀스 검증 (Protocol Context Audit)

> [!IMPORTANT]
> **V&V Finding 3: `CONNECT_REQUEST` 메시지의 시퀀스 편입 권고**
> * **기술적 사실:** 현재 시스템은 `CONNECT_REQUEST`를 시퀀스 통제 체계에 포함하지 않는(Unsequenced) 구조로 구현되어 있습니다. (프로젝트 이력: `TC-PC-026`, `RV-292`로 종결됨)
> * **개선 권고:** 프로젝트 내부의 관리적 종결(Closeout)과 별개로, SIL4 통신 인증에서는 최초 연결 요청부터 흐름 제어(Flow Control)의 연속성이 요구됩니다. 향후 해당 메시지를 시퀀스 기반 체계로 편입시키는 방향으로 개선을 권고합니다.

---

### V&V 결론 (Phase 1)

현재 코드는 프로젝트 베이스라인을 충실히 따르고 있으나, RaSTA 표준과 대조 시 상태 전이 및 초기 시퀀스 관리 영역에서 일부 불일치가 존재합니다. 원활한 외부 SIL4 인증 절차를 위해 식별된 항목들에 대한 구조적 개선이 필요합니다.
