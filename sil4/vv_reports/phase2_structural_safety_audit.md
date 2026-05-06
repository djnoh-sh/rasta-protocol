# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.4 (V&V Independent Re-assessment - Objective)
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
