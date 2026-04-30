# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.2 (Updated to reflect Official Baseline 2026-04-30)
**Target Component:** 전체 시스템 기능 요구사항 (RaSTA Protocol Specification)
**Focus Area:** Functional Completeness & Requirement Traceability (Phase 4)

V&V(검증 및 확인)의 핵심 목표는 **요구사항 명세서(Requirements Specification)에 정의된 모든 기능이 빠짐없이 올바르게 구현되었는가(Validation)**를 객관적으로 증명하는 데 있습니다.

본 4단계 보고서는 RaSTA 표준(DIN VDE V 0831-200)의 핵심 기능적 요구사항을 기준으로, 현재 프로젝트에서 확정된 '베이스라인 스코프(Baseline Scope)' 대비 현재 구현된 코드베이스가 얼마나 완전한지(Completeness) 평가한 결과입니다.

---

## 1. 구현이 완료된 정상 기능 (Implemented Features)

현재 시스템은 통신 세션의 생명주기를 관리하는 '코어 프로토콜(Core Protocol)' 관점에서 현행 베이스라인의 representative scope를 충족합니다.

> [!TIP]
> **V&V Finding 14: 기초 연결 관리 및 세션 생명주기 (PASS)**
> * 3-way Handshake(연결 설정) 베이스라인 흐름 제어 완료
> * Sequence Number 발급 및 수신 패킷의 Confirmation 검증 로직
> * `Supervision Timer`를 활용한 연결 감시(Heartbeat) 및 타임아웃 처리
> * Sequence Gap 발생 시 재전송(Retransmission) 계류 및 복구(Recovery) 절차
> * 통신 단절 시 Fail-Safe 상태(`SAFE_DISCONNECT`) 진입 로직

> [!TIP]
> **V&V Finding 15: 다중 채널(Redundancy) 정책 관리 (Representative Closeout)**
> * **상황:** 다중 채널(Primary/Secondary)을 관리하는 `rsrx_channel_manager.c`가 구현되어 있습니다. Active-Standby 전환, Holdoff 시간, Flap Penalty 적용 등 다중 채널 관리 로직의 뼈대가 마련되어 있습니다.
> * **V&V 공식 의견:** 현행 베이스라인 체계 하에서 현재의 다중 채널 관리 기능은 **'대표적 종결(Representative Closeout)'** 상태로 취급되며 현 범위의 기능적 구현 범위를 달성했습니다. 이는 최종 redundancy policy 완성을 의미하지 않으며, 향후 보다 정교한 라우팅 체계, long-run generalization, 다양한 모드 확장이 `R-003` future growth로 남아 있습니다.

---

## 2. 향후 릴리스 확장을 위한 로드맵 요구사항 (Future Parity Growth)

SIL4 통신의 필수 조건인 **데이터 무결성(Data Integrity) 및 보안성(Security)** 측면에서 규격이 요구하는 일부 기능은 현재 베이스라인 스코프에 포함되지 않았으며, 공식 로드맵의 향후 확장 과제(`R-006`)로 관리되고 있습니다.

> [!NOTE]
> **V&V Finding 16: 메시지 무결성 보호(MAC/CRC) 이식 (Roadmap R-006)**
> * **규격 요구사항:** RaSTA 패킷은 전송 중 데이터 훼손(Corruption) 및 위변조(Spoofing)를 막기 위해 암호화 해시(MD4 MAC 등)와 CRC를 PDU(Protocol Data Unit) 하단에 덧붙여야 합니다.
> * **현재 구현 상태:** 간소화된 결정론적 스켈레톤 코덱(`rsrx_codec.c`)으로 동작 중입니다.
> * **V&V 공식 의견:** 이는 현재 베이스라인 내의 결함이 아니라, 향후 안전 인증 획득을 위해 반드시 완수되어야 할 **미래 로드맵 확장 과제(Future Growth)**로 올바르게 식별되어 있습니다. 

> [!NOTE]
> **V&V Finding 17: 타임스탬프(Timestamp) 및 Replay 방어 이식 (Roadmap R-006)**
> * **규격 요구사항:** 재전송 공격(Replay Attack)과 패킷 지연(Delay) 방지를 위해 패킷 발송 시간(Timestamp) 기록과 수신 허용 윈도우(Time Window) 검증이 필요합니다.
> * **현재 구현 상태:** PDU 구조체 및 코덱에 Timestamp 로직이 미구현 상태입니다.
> * **V&V 공식 의견:** Finding 16과 동일하게, 최종 상호 운용성을 위해 베이스라인 이후 단계에서 `R-006`에 따라 추가되어야 할 필수 보안 모듈입니다.

---

### V&V 결론 (Phase 4)

현재 시스템은 베이스라인이 목표로 삼은 **상태 관리, 흐름 제어, 그리고 기초적인 다중 채널 관리(Redundancy)** 영역을 representative closeout 상태로 정리했습니다. 

비록 보안 및 무결성 계층(MAC/CRC/Timestamp)이 아직 구현되지 않아 당장의 SIL4 안전 인증 심사를 통과할 수는 없으나, 해당 누락 요소들이 숨겨진 버그(Defect)가 아니라 프로젝트 로드맵(`R-006`) 하에 **통제 가능한 미래 개발 항목(Future Growth)**으로 명확히 관리되고 있다는 점은 프로젝트 관리적 관점에서 매우 바람직합니다.
