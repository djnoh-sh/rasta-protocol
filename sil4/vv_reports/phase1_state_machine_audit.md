# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.5 (V&V Independent Re-assessment - Objective)
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

---

## 3. 2026-05-21 상태 추적 및 종결 (V&V Follow-Up Audit - v1.5)

최근 완료된 프로토콜 컨텍스트 및 상태 머신 개선 사항을 검토하여 기존 지적 사항들의 조치 결과를 아래와 같이 업데이트합니다.

### Finding 1: 3-way Handshake 연결 성립 절차 규격 호환성 개선 권고
* **조치 결과:** `[RESOLVED/CLOSED]`
* **기술적 사실 및 근거:** 해당 항목은 개발팀 및 프로젝트 공식 입장(`OFFICIAL_RESPONSE_TO_VV_REPORTS_2026-04-29.md`)에 따라 "프로토콜 일치성 해석 후보군"으로 분류되었으며, 현재 베이스라인 스펙의 동작 범위로 유지하기로 최종 합의되었습니다. 회귀 방지 대책으로 `HANDSHAKE_SUCCESS` 상태 전이 시 세부 액션(Action list)에 대한 검증 밀도를 대폭 보강하는 단위 테스트가 추가 완료되었습니다(`RV-291` 완료). 이에 따라 본 감사 단계에서는 종결 처리합니다.

### Finding 2: Server의 Start 상태 내 Heartbeat 수신 처리 개선 권고
* **조치 결과:** `[RESOLVED/CLOSED]`
* **기술적 사실 및 근거:** Finding 1과 동일하게 공식 프로젝트 스펙 해석 합의에 따라 현 베이스라인 동작(Server의 Start 상태 내 수신 규칙)이 타당한 계약(Contract)으로 인정되었습니다. 이에 대응하여 상태 전이 액션 검증 어서션 밀도가 확보되었으므로(`RV-291` 완료) 본 항목을 종결 처리합니다.

### Finding 3: `CONNECT_REQUEST` 메시지의 시퀀스 편입 권고
* **조치 결과:** `[RESOLVED/CLOSED (Baseline Decision)]`
* **기술적 사실 및 근거:** 현 베이스라인에서 `CONNECT_REQUEST`는 의도적으로 시퀀스 제어 체계에 편입하지 않는(Unsequenced) 형태로 설계되어 있음이 확인되었습니다. 이 설계 의사결정은 공식 형상 통제 하의 검토 기록(`RV-292`)을 통해 확정되었으며, 향후 추가 규격 확장 단계(R-001)에서 관리되도록 분류되었습니다. 현재 기준선에 적합하게 문서화 및 형상 정리가 완료되었으므로 종결 처리합니다.

---

## 4. 2026-05-26 상태 추적 및 업데이트 (V&V Follow-Up Audit - v1.6)

세션 리셋 및 재시작 프로토콜의 안정성 보강 작업을 검토한 결과는 다음과 같습니다.

### 세션 리셋(Reset) 및 초기화(Init) 시의 리포트 클리어 보완
* **기술적 사실 및 근거:** 기존 구현에서는 세션 리셋(`rsrx_session_reset`) 이후에도 이전 트랜지션 및 액션 이력(`xLastReport`)이 완전히 클리어되지 않는 잔여 데이터 리스크가 존재했으나, 리셋 성공 시 `vResetLastReport` 헬퍼 함수를 통해 리포트 구조체를 명시적으로 초기화하도록 개선되었습니다. 이에 대한 단위 테스트(`RV-395`) 및 프로토콜 컨텍스트 초기화 가드 테스트(`RV-391`, `RV-392`)가 추가되어 리셋 거동의 일관성이 검증되었습니다.

### 리셋 후 세션 재시작 및 재결합 흐름 보완
* **기술적 사실 및 근거:** 상태 머신이 리셋 상태로 귀결된 이후 세션을 재시작(Session restart after reset)할 때 발생할 수 있는 전이 예외를 방어하기 위해, 상태 머신 리셋 베이스라인 테스트(`RV-393`) 및 세션 재시작 통합 테스트(`RV-396`, `RV-397`)가 보강되어 정상 작동을 확인했습니다.

* **V&V 평가:** 기존의 CLOSED 상태를 양호하게 유지하며, 상태 머신의 경계 조건(리셋 후 재시작)에 대한 강건성과 리포팅 신뢰성이 객관적 테스트 및 코드 개선을 통해 확보되었음을 확인했습니다.

---

## 5. [V&V Backlog] 차기 마일스톤 진입 시 검증 필수 요구사항

*   **SafeRTOS 이식성 검증 단계:** SafeRTOS 환경 이식 시 태스크 컨텍스트 내에서의 세션 상태 머신 Failsafe 전이 거동 및 인터럽트 서비스 루틴(ISR)과의 상태 동기화 설계 명세 확인.


