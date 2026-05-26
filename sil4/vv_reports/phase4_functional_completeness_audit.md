# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.5 (V&V Independent Re-assessment - Objective)
**Target Component:** 전체 시스템 기능 요구사항 (RaSTA Protocol Specification)
**Focus Area:** Functional Completeness & Requirement Traceability (Phase 4)

본 보고서는 구현된 코드베이스가 RaSTA 표준(DIN VDE V 0831-200)의 핵심 기능적 요구사항을 충족하는지 객관적 관점에서 평가한 결과입니다. 

---

## 1. 구현 기능 평가 (Implemented Features)

> [!TIP]
> **V&V Finding 9: 다중 채널(Redundancy) 기본 구조 확보**
> * **기술적 사실:** 다중 채널 관리 로직(`rsrx_channel_manager.c`)의 기초 체계(Active-Standby 전환, Flap Penalty 등)가 마련되어 있으며, 프로젝트 베이스라인의 Representative Closeout 요건을 충족합니다.
> * **개선 권고:** 향후 다중 경로 라우팅 요구사항에 대비하여 아키텍처의 점진적 발전을 권고합니다.

---

## 2. 보안 및 무결성 기능 확장 요구 (Future Parity Growth)

> [!WARNING]
> **V&V Finding 10: 보안/무결성 확보를 위한 코덱 구조 개선 권고**
> * **기술적 사실:** 데이터 무결성 검증(MAC, CRC) 및 Replay 방지용 타임스탬프(Timestamp) 로직이 현재 구현 구조에서 제외되어 있습니다. (내부 로드맵 `R-006` 과제로 분류됨)
> * **개선 권고:** 해당 기능 부재는 통신 무결성에 영향을 미치므로, 최종 SIL4 안전 인증 이전에 보안 모듈 구현이 선행될 것을 권고합니다.

---

### V&V 결론 (Phase 4)

현재 코드는 프로토콜 상태 관리 측면에서 프로젝트의 베이스라인 스코프를 만족합니다. 데이터 무결성 확보(`R-006` 과제)가 최종 SIL4 인증을 위한 주요 잔여 요구사항으로 식별됩니다.

---

## 3. 2026-05-21 상태 추적 및 종결 (V&V Follow-Up Audit - v1.5)

무결성 검증 모듈 추가 및 이중화 검증 범위 재평가에 따른 지적 조치 결과는 다음과 같습니다.

### Finding 9: 다중 채널(Redundancy) 기본 구조 확보
* **조치 결과:** `[COMPLIANT (Representative Closeout)]`
* **기술적 사실 및 근거:** 현재 구현된 액티브-스탠바이 전환, Holdoff 지연(최대 20), Flap Penalty 제어(최대 19), Terminal Outcome 전이(최대 19) 및 토폴로지 검증 로직은 현 프로젝트 베이스라인 하에서 모든 테스트 케이스 통과가 입증되었습니다. 멀티패스 라우팅 등의 추가 이중화 모드는 현 단계의 필수 불완전 요소가 아닌, 향후 로드맵(`R-003`)에서 계획된 확장 기능으로 정의되었음을 재확인했습니다.

### Finding 10: 보안/무결성 확보를 위한 코덱 구조 개선 권고
* **조치 결과:** `[RESOLVED/CLOSED (CRC32 Implemented & Diagnostics Extended)]`
* **기술적 사실 및 근거:** 
  1. **CRC32 무결성 가드 탑재:** CRC32 검증을 지원하는 코덱 포트 API(`rsrx_codec_get_crc32_port`)가 신규 구축되었으며, 수신 프레임의 CRC32 불일치 및 길이 검증 로직이 소스코드 및 통합 테스트(`RV-332`~`RV-340`, `RV-341`~`RV-346`) 상에 성공적으로 연동되었습니다.
  2. **진단(Diagnostic) 세분화:** `rsrx_codec_status_t` 구조에 `CRC_MISMATCH`, `CRC_TRUNCATED`, `RESERVED_HEADER_NONZERO`, `LENGTH_MISMATCH` 등의 상태 카테고리가 구현되었으며, 파싱 오류 시 부분 디코딩된 출력 버퍼가 강제 초기화되도록 방어벽이 마련되었습니다(`RV-347`~`RV-378`).
  3. **보안 게이팅:** CRC 요구 설정에 따른 API 초기화 거부 정책 및 지원되지 않는 보안 프로파일 요청 시의 거절 로직도 검증이 완비되었습니다. 
  나머지 무결성 필드(MAC, 타임스탬프 등)는 로드맵 `R-006`에 명시된 향후 보안 확장 단계에 따라 추적되므로, 현 3-4단계 감사 관점에서는 본 항목을 종결 처리합니다.

---

## 4. 2026-05-26 상태 추적 및 업데이트 (V&V Follow-Up Audit - v1.6)

RaSTA SR 규격 부합화 사전 설계 및 이중화 제어 기능 보완에 대한 검토 결과는 다음과 같습니다.

### RaSTA SR PDU 규격 기초 설계 및 매핑 인터페이스 구축
* **기술적 사실 및 근거:** RaSTA SR(Safety Relation) 메시지 규격 준수를 위한 PDU 와이어 프로파일 설계 초안이 마련되었습니다(`RV-399`, `RV-400`). 이를 실현하기 위해 `rsrx_codec.h`에 28바이트의 RaSTA SR 헤더 크기(`D_RSRX_CODEC_RASTA_SR_HEADER_BYTES`) 및 8바이트 타임스탬프 규격을 정립하고, `rsrx_codec_get_rasta_sr_wire_profile`를 추가했습니다.
* **프로토콜 상호 매핑 구현:** 내부 프로토콜 메시지 타입과 RaSTA SR PDU 타입(6200U~6241U) 간의 상호 변환을 지원하는 매핑 함수 및 오류 발생 시 내부 원인 코드와 RaSTA SR Disconnect Reason 간의 연동 함수가 구현되었습니다(`RV-401`, `RV-402`).

### 다중 채널(Redundancy) 관리 신뢰성 보완
* **기술적 사실 및 근거:** 채널 매니저 내부적으로 Flap penalty 누적 시의 Saturation(포화) 임계치 한계 동작을 방어하기 위한 로직이 엄격화되었습니다(`RV-381`). 또한 리셋 감사 정보 보존(`RV-387`) 및 재전송 제어 시의 격리 동작(`RV-386`) 보완이 완료되었습니다.

* **V&V 평가:** 비록 실제 전송 단에서의 RaSTA SR 패킷 완전 인코딩/디코딩 동작은 향후 과제(`R-006` 잔여분)로 분류되나, 프로토콜 및 코덱 수준에서 RaSTA SR Numeric 규격을 이식하기 위한 인터페이스 계약 및 데이터 매핑 로직이 선행 구축되어 규격 일치성(Parity) 완성도가 크게 향상되었음을 확인했습니다.

