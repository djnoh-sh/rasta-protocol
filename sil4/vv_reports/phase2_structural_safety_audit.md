# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.5 (V&V Independent Re-assessment - Objective)
**Target Component:** `rsrx_codec.c`, `rsrx_api.c`, `rsrx_orchestrator.c` 외 전체 소스 코드
**Focus Area:** Structural Safety, Memory Management & Defensive Programming (Phase 2)

본 보고서는 대상 코드가 메모리 안전성 및 MISRA-C 코딩 가이드라인을 이행하고 있는지 기술적 사실을 바탕으로 평가한 2단계 V&V 검증 결과입니다.

---

## 1. 메모리 관리 및 할당 안전성 (Memory Safety)

> [!TIP]
> **V&V Finding 4: 동적 메모리 할당 배제 (COMPLIANT)**
> * **기술적 사실:** `sil4/src/` 전체 코드에서 `malloc`, `free` 계열의 동적 할당 함수가 사용되지 않았습니다. 
> * **개선 권고:** 외부 인증 기관 제출을 대비하여, 스택 사용량 상한(Stack Bound) 분석 리포트 및 정적 메모리 매핑(Memory Map) 증빙 자료 보완을 권고합니다.

---

## 2. 방어적 코딩 및 포인터 검증 (Defensive Programming)

> [!TIP]
> **V&V Finding 5: 포인터 검증 및 버퍼 오버플로우 방어 (COMPLIANT)**
> * **기술적 사실:** API 진입부에서 널(NULL) 체크가 수행되며, 패킷 처리 시 용량(Capacity) 검증이 선행되고 있습니다. 

---

## 3. 컴파일러 경고 강화 (Build-Hardening)

> [!WARNING]
> **V&V Finding 6: 엄격한 타입 형변환(Casting) 경고 옵션 정규화 권고**
> * **기술적 사실:** 암시적 형변환(Implicit Casting) 검출을 위해 CMake 설정에 `RSRX_ENABLE_STRICT_WARNING_HARDENING` 기반의 경고 프로파일이 구축되어 있습니다.
> * **개선 권고:** 해당 옵션을 일회성으로 사용하지 않고 지속 통합(CI) 환경 및 릴리스 파이프라인의 필수 통과 기준(Default Gate)으로 적용할 것을 권고합니다.

---

### V&V 결론 (Phase 2)

현재 아키텍처는 동적 할당 배제 및 방어적 코딩 관점에서 SIL4 메모리 관리 요구사항을 기본적으로 충족합니다. 완전한 외부 심사 대비를 위해서는 스택 분석 등의 추가 증빙 자료 확보와 빌드 게이트 강화가 요구됩니다.

---

## 4. 2026-05-21 상태 추적 및 종결 (V&V Follow-Up Audit - v1.5)

컴파일러 엄격한 경고 설정 및 메모리 사용 증빙 고도화 결과를 반영하여 기존 권고 사항들의 조치 결과를 아래와 같이 업데이트합니다.

### Finding 4: 동적 메모리 할당 배제
* **조치 결과:** `[COMPLIANT (Evidence Completed)]`
* **기술적 사실 및 근거:** 정적 메모리 할당 원칙은 소스 코드 전체에 대해 준수되고 있습니다. 증빙 보완 권고에 부합하여, 스택 사용량 분석 및 링커 맵 파일 생성 규칙(`RSRX_ENABLE_STACK_MEMORY_EVIDENCE` 옵션 추가)이 2026-05-06에 규격화되었습니다. 호스트 환경의 스택 사용량 증빙 리포트 및 대표 링커 맵이 `sil4/docs/evidence/reports/stack_memory_host_baseline_2026-05-06/` 하위에 최종 정합 완료되었습니다.

### Finding 5: 포인터 검증 및 버퍼 오버플로우 방어
* **조치 결과:** `[COMPLIANT]`
* **기술적 사실 및 근거:** API 내부 진입 시의 NULL 체크가 상시 수행 중이며, 이번 무결성 검증 추가 단계(`RV-332`~`RV-380`)에서도 모든 공개 API(`rsrx_api.c`, `rsrx_channel_manager.c`, `rsrx_transport_supervisor.c`)의 포인터 가드 및 상태 가설 매트릭스 테스트 커버리지가 확보되었습니다.

### Finding 6: 엄격한 타입 형변환(Casting) 경고 옵션 정규화 권고
* **조치 결과:** `[RESOLVED/CLOSED (Policy Decision Complete)]`
* **기술적 사실 및 근거:** 엄격한 타입 형변환 경고 옵션(`-Wconversion`, `-Wsign-conversion`)의 CI default-gate 정규화는 2026-05-06에 공식 검토회의(`RV-331`)를 통해 `Evidence-only` 정책으로 합의 및 클ローズ되었습니다. 컴파일러 버전 변화에 따른 불필요한 빌드 차단을 방지하되, 주기적 릴리스 증빙 단계에서 정적 분석 자료를 제출하는 것으로 최종 정책(`EVID-CI-118`)이 정립되었습니다. 현 기본 빌드도 `-Wall -Wextra -Werror` 게이트 하에 경고 0건으로 통과하고 있으므로, 본 항목은 종결 처리합니다.

---

## 5. 2026-05-26 상태 추적 및 업데이트 (V&V Follow-Up Audit - v1.6)

하드웨어 결합성 개선을 위한 Seam 도입 및 API 진입점 가드 강화에 대한 검토 결과는 다음과 같습니다.

### 코덱 계층 내 CRC32 계산기 주입 구조 도입 (Dependency Injection)
* **기술적 사실 및 근거:** 하드웨어 가속기(TI AM263Px Hardware CRC Adapter 등)와의 연동을 대비하여 코어 코드의 수정 없이 외부 함수를 주입할 수 있는 `rsrx_codec_encode_message_with_crc32_calculator` 및 `rsrx_codec_decode_frame_with_crc32_calculator` 인터페이스가 도입되었습니다(`RV-383`, `RV-384`). 이 과정에서 포인터 주입 시의 NULL 가드 검증도 완료되어 구조적 안전성이 유지됨을 확인했습니다.

### 프로토콜 컨텍스트 API 진입점 가드 강화
* **기술적 사실 및 근거:** 프로토콜 컨텍스트의 다양한 런타임 제어 API 호출 시 발생할 수 있는 데이터 오염을 방지하기 위해 가드 아키텍처가 확장되었고, 이에 대응하는 안정성 리뷰가 완료되었습니다(`RV-385`, `RV-390`).

* **V&V 평가:** 메모리/구조 안전성에 대한 기존 [COMPLIANT] 상태를 완벽히 충족하며, 플랫폼 독립적 구조를 유지하면서도 물리적 하드웨어 이식성을 향상시키기 위한 인터페이스 Seam이 설계 원칙에 부합하게 구현되었습니다.

---

## 6. 2026-06-02 상태 추적 및 업데이트 (V&V Follow-Up Audit - v1.7)

상세 설계와 구현부 간의 메모리 한계성 및 오버플로우 방어 조건에 대한 검토 결과는 다음과 같습니다.

### Channel Manager의 preferred recovery 수치 한계 포화 연산(Saturation) 검증
*   **기술적 사실 및 근거:** `channel_manager_lld_draft.md`에 기술된 "holdoff target 계산 시 `UINT32_MAX` 오버플로우 포화 처리" 규칙이 `rsrx_channel_manager.c`의 `uGetEffectiveHoldoffTarget` 함수 내에 정수 오버플로우 방어벽(`(UINT32_MAX - uTarget) < pending_penalty`)과 결합되어 완벽하게 포화값(`UINT32_MAX`)을 결정적으로 반환하도록 설계-구현 정합성이 지켜지고 있음을 검증했습니다.

### Unsigned 상수 접미사(U) 적용 상태 수동 검산
*   **기술적 사실 및 근거:** `CODING_RULES.md`의 "Unsigned 정수 상수에 `U` 접미사 필수 사용" 규정에 따라, `rsrx_state_machine.c`, `rsrx_channel_manager.c`, `rsrx_codec.c` 내부의 모든 Unsigned integer 상수 선언부(`0U`, `1U`, `2U`, `4U`, `16U`, `0xFFU` 등)에 `U` 접미사가 누락 없이 정상 적용되어, 데이터 형 불일치로 인한 오작동 리스크를 차단함을 확인했습니다.

### [V&V Finding E] 공개 API 계층의 비동기 호출 간 임계 영역(Critical Section) 보호 부재
*   **기술적 사실 및 근거:** `rsrx_api.c` 내부의 `rsrx_session_send_application_data` 등 공개 API는 전송 실패 등의 상황 시 `vNotifyDirectReject`를 호출해 세션 데이터 `xLastReport`를 직접 갱신합니다. 만약 SafeRTOS 멀티태스킹 환경에서 전송 API 호출과 백그라운드 수신 이벤트 처리가 서로 다른 태스크에서 비동기로 병행될 경우, 동일 세션 및 리포트 데이터에 대한 **Data Race(데이터 경쟁)** 상태가 유발되어 상태가 오염될 잠재적 위험이 있습니다. `rsrx_platform.h`에 명시된 임계 영역 가드를 모든 공개 API 및 핸들러 진입점에 보호막으로 추가 적용할 것을 강력 권고합니다.

---

## 7. [V&V Backlog] 차기 마일스톤 진입 시 검증 필수 요구사항

*   **타깃 이식성 검증 단계:** 외부에 주입될 하드웨어 CRC32 연산 어댑터와 portable C 코덱 간의 인터페이스 안전성(포인터 가드) 재검증 및 타깃 컴파일 빌드 기준의 최종 스택 바운드/링커 맵 메모리 분석 증빙 확보.



