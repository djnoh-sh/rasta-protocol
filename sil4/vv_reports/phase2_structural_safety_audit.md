# SIL4 Verification & Validation (V&V) Report: RaSTA Protocol

**Document Version:** 1.2 (Updated to reflect Official Baseline 2026-04-30)
**Target Component:** `rsrx_codec.c`, `rsrx_api.c`, `rsrx_orchestrator.c` 외 전체 소스 코드
**Focus Area:** Structural Safety, Memory Management & Defensive Programming (Phase 2)

본 보고서는 작성된 C 소스 코드가 메모리 안전성, 포인터 연산, 그리고 MISRA-C 코딩 가이드라인을 얼마나 준수하고 있는지(정적 분석 및 구조적 관점) 평가한 2단계 V&V 검증 결과입니다.

---

## 1. 메모리 관리 및 할당 안전성 (Memory Safety)

SIL4 소프트웨어는 예기치 않은 메모리 고갈이나 단편화(Fragmentation)를 막기 위해 동적 메모리 할당(Dynamic Memory Allocation)을 절대적으로 금지합니다. (MISRA-C:2012 Dir 4.12)

> [!TIP]
> **V&V Finding 5: 동적 메모리 할당 배제 (COMPLIANT WITHIN CURRENT BASELINE)**
> * **검사 결과:** 전체 `sil4/src/` 하위 소스 코드에서 `malloc`, `calloc`, `realloc`, `free` 계열의 표준 라이브러리 함수 호출이 **단 한 건도 발견되지 않았습니다.**
> * **평가:** 모든 상태 및 큐(Queue), 세션 데이터가 정적으로 할당된 Context 구조체 내부에 관리되고 있어 현 베이스라인의 동적 할당 배제 기준을 충족합니다. 단, 최종 SIL4 메모리 증빙에는 stack bound, linker/memory map, recursion 배제, vendor/MISRA evidence 같은 외부 증빙이 추가로 필요합니다.

---

## 2. 방어적 코딩 및 포인터 검증 (Defensive Programming)

Null Pointer Dereference는 시스템 크래시를 유발하는 가장 흔한 요인입니다.

> [!TIP]
> **V&V Finding 6: 포인터 유효성 검사 (COMPLIANT)**
> * **검사 결과:** `rsrx_api.c`, `rsrx_codec.c` 등 모든 외부 노출 API의 최상단에서 입력 포인터에 대한 명시적인 널 체크(`pxSession == (rsrx_session_t *)0`)가 수행되고 있습니다.
> * **평가:** 예외 처리 없이 즉각적으로 에러 상태(`RSRX_STATUS_INVALID_ARGUMENT`)를 반환하거나 함수를 종료하는 방어적 프로그래밍 원칙이 철저히 지켜졌습니다.

> [!TIP]
> **V&V Finding 7: 문자열 함수 및 버퍼 오버플로우 방어 (COMPLIANT)**
> * **검사 결과:** `strcpy`, `strcat`, `sprintf`와 같은 불안전한 문자열 복사 함수가 사용되지 않았습니다. `rsrx_codec.c`의 인코딩/디코딩 로직에서도 버퍼 오버플로우를 막기 위해 항상 Capacity 체크(`pxBuffer->xBufferCapacity < xRequiredBytes`)와 최대 페이로드 사이즈 검증(`pxRequest->xPayloadLength > D_RSRX_CODEC_MAX_PAYLOAD_BYTES`)을 선행하고 있습니다.

---

## 3. MISRA-C:2012 주요 룰 준수 여부 (MISRA-C Compliance)

> [!NOTE]
> **V&V Finding 8: 엄격한 데이터 타입 명시 (COMPLIANT)**
> * **검사 결과:** `int`, `long`, `char` 등의 시스템 종속적인 기본 타입을 배제하고, `stdint.h` 기반의 명시적 길이 지정 타입(`uint32_t`, `uint16_t`, `uint8_t`)을 전면 적용하여 이식성(Portability)과 안전성을 높였습니다. (MISRA Dir 4.6)

> [!WARNING]
> **V&V Finding 9: Magic Number 사용 점검 및 타입 캐스팅 (Accepted Build-Hardening Closeout / R-007)**
> * **검사 결과:** 대부분의 상수들이 `#define` 형태의 매크로로 잘 정의되어 있으나, 일부 로직에서 리터럴 정수에 대한 무분별한 암시적 형변환(Implicit Casting) 위험이 존재할 수 있습니다.
> * **처리 상태:** 은연중에 발생하는 부호(Sign) 변환이나 잘림(Truncation)을 빌드 단계에서 평가하기 위한 **CMake** 기반 strict warning profile이 `RSRX_ENABLE_STRICT_WARNING_HARDENING` 옵션으로 추가됐고, `SA-REP-006` 및 `RV-293` 기준으로 closeout 되었습니다. 향후 이를 기본 gate로 승격할지는 별도 정책 결정입니다.

---

### V&V 결론 (Phase 2)

신규 SIL4 아키텍처는 **동적 할당 배제, 엄격한 널 포인터 체크, 버퍼 오버플로우 방어** 측면에서 강한 구조적 안전성을 보입니다. 다만 최종 SIL4/MISRA 적합성 주장은 현재의 코드 구조와 cppcheck/compiler evidence만으로 완결되지 않으며, `R-005`에 남아 있는 vendor evidence와 외부 증빙 확보가 필요합니다.
