# SIL4 RaSTA Reimplementation: Comprehensive V&V Audit Report

**Audit Date:** 2026-06-02  
**Target Baseline:** Repository state as of 2026-06-02  
**Audit Scope:** 
- Source Code: `sil4/src/*`, `sil4/include/*`
- Design Specifications: LLD Drafts (`lld/*`), HLD Drafts (`hld/*`), Normative Features (`INV-RASTA-001`)
- Verification Specifications: Test Specifications (`verification/*`), Unit/Integration Tests (`tests/*`)
- Safety & Quality Rules: `CODING_RULES.md`, `SIL4_REIMPLEMENTATION_RULES.md`, `traceability_matrix_initial.md`

---

## 1. Executive Summary (요약)

본 V&V 감사(Audit)는 단순한 최근 패치 확인을 넘어, 요구사항-설계-코드-테스트 및 코딩 표준에 이르는 SIL4 재구현 프로젝트 전반의 무결성(Integrity)과 정합성(Alignment)을 교차 검증하기 위해 독립적인 관점에서 수행되었습니다. 

검사 결과, 전체 소스 코드와 상세 설계(LLD) 문서 간의 정합성은 기본 전이 테이블 기준 99% 이상 일치하여 매우 양호합니다. 그러나 비동기 멀티태스킹(SafeRTOS) 타깃 이식 및 안전 임베디드 관점에서 심층 코드 리뷰를 수행한 결과, **개발팀이 스스로 식별하지 못한 치명적인 데이터 경쟁(Data Race) 위협 및 타이머 오작동 관련 잠재 결함 2건**을 추가로 발굴하였습니다.

식별된 취약점과 문서 결함 1건에 대한 상세 팩트(Fact) 및 조치 권고 사항을 다음과 같이 보고합니다.

---

## 2. Key V&V Audit Findings (상세 감사 결과)

### [Finding E] 공개 API 계층의 비동기 호출 간 임계 영역(Critical Section) 보호 부재 (잠재적 Data Race 위협)
*   **기술적 사실:**
    *   `[rsrx_api.c](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/src/rsrx_api.c)` 내의 `rsrx_session_send_application_data` 등 공개 API는 전송 실패 시 `vNotifyDirectReject`를 호출하여 세션의 공유 상태 데이터인 `xLastReport`를 직접 수정하고 등록된 콜백을 실행합니다.
    *   현재 소스 코드 상에는 API 진입점들에 대한 뮤텍스(Mutex) 또는 크리티컬 섹션(Critical Section) 보호 장치가 설계되어 있지 않습니다.
*   **V&V 평가 및 권고:**
    *   비동기 타깃 환경(SafeRTOS 멀티태스킹 등)에서 전송 API 호출과 백그라운드의 네트워크 수신 이벤트 처리(`eProcessSessionEvent`)가 서로 다른 태스크 및 인터럽트 서비스 루틴(ISR)에서 병행 실행될 경우, 동일 세션 컨텍스트 및 `xLastReport`에 대한 **동시 접근 경쟁 상태(Data Race)**가 유발되어 내부 상태가 오염(Corruption)될 수 있습니다.
    *   `rsrx_platform.h`에 정의된 플랫폼 임계 영역 인터페이스를 활용하여 데이터 쓰기가 일어나는 모든 공개 API 및 이벤트 핸들러 진입점에 안전 잠금(Locking) 메커니즘을 긴급 적용할 것을 권고합니다. (안전 무결성 통과 필수 조치)

### [Finding F] 세션 리셋(`rsrx_session_reset`) 시 실행 중인 타이머의 명시적 비활성화(Stop) 누락 우려 (잠재적 오작동 결함)
*   **기술적 사실:**
    *   `[rsrx_api.c](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/src/rsrx_api.c)`의 `rsrx_session_reset` 함수(Line 516-542)는 채널 매니저와 오케스트레이터를 리셋하지만, 기존 런타임에 작동 중이던 물리/가상 타이머(Supervision, Retransmission Timer 등)를 비활성화하는 명시적인 플랫폼/타이머 정지 API 호출이 누락되어 있습니다.
*   **V&V 평가 및 권고:**
    *   만약 타이머가 기동 중인 상태에서 세션만 리셋될 경우, 리셋 직후(초기화되지 않은 대기 상태 등) 타이머 만료 인터럽트가 유입되어 상태 기계에 `RSRX_EVENT_TIMEOUT` 등이 주입되면서 예기치 않은 오동작이나 Failsafe 불일치를 유발할 수 있습니다.
    *   세션 리셋 진입 즉시 런타임 타이머들을 명시적으로 정지(Stop/Cancel)시키는 안전 가드 로직을 추가할 것을 권고합니다.

### [Finding A] 테스트 사양서(TS-002) 내 테스트 케이스 ID 중복 오류 (Document Defect)
*   **기술적 사실:**
    *   `[connection_state_machine_test_spec_draft.md](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/docs/verification/connection_state_machine_test_spec_draft.md)` (TS-002) 문서의 테스트 케이스 표에서 동일한 ID인 `TC-SM-012`가 두 개의 서로 다른 검증 목적에 중복 매핑되어 있음이 확인되었습니다.
    *   *Line 44:* `TC-SM-012` -> "repeated retransmission gap 검증"
    *   *Line 51:* `TC-SM-012` -> "`SHUTDOWN` 상태 입력 무시 검증"
*   **V&V 평가 및 권고:**
    *   단순 오기로 파악되나, SIL4 등급의 안전 소프트웨어 추적성 매트릭스(Traceability Matrix) 연계 시 ID 중복으로 인한 일대일 매핑 무결성이 깨지게 되므로, `SHUTDOWN` 관련 케이스의 ID를 `TC-SM-019` 등으로 변경하여 고유화할 것을 권고합니다. (동작 코드에는 영향이 없으며 문서 수정만 필요)

### [Finding B] Channel Manager Holdoff 포화 연산 정합성 (COMPLIANT)
*   **기술적 사실:**
    *   상세 설계서 `[channel_manager_lld_draft.md](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/docs/design/lld/channel_manager_lld_draft.md)`의 Line 79에는 "base holdoff와 pending penalty의 합이 `UINT32_MAX`를 초과할 때 wraparound되지 않고 포화(Saturation)되어야 한다"는 명시적 규칙이 존재합니다.
    *   실제 소스 코드 `[rsrx_channel_manager.c](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/src/rsrx_channel_manager.c)`의 `uGetEffectiveHoldoffTarget` 함수(Line 3-15)는 다음과 같이 정수 오버플로우 가드(`UINT32_MAX - uTarget`)와 결합되어 완벽히 포화값(`UINT32_MAX`)을 반환하도록 정합 구현되었습니다.

### [Finding C] Codec 디코딩 시 NULL Argument 입력에 대한 조기 버퍼 클리어 정책 (COMPLIANT)
*   **기술적 사실:**
    *   상세 설계서 `[protocol_codec_lld_draft.md](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/docs/design/lld/protocol_codec_lld_draft.md)`의 Line 49 및 Line 63에 정의된 "null 포인터 수신 시 INVALID_ARGUMENT로 거부 전 출력 버퍼를 클리어한다"는 방어적 정책이 `[rsrx_codec.c](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/src/rsrx_codec.c)` 내의 `rsrx_codec_decode_frame`을 비롯한 모든 디코딩 진입점에 일관되게 적용(`vClearDecodedMessage` 우선 호출 후 NULL 체크)되어 구현되었음을 검증했습니다.

### [Finding D] Unsigned 상수 접미사(U) 적용 상태 수동 감사 (COMPLIANT)
*   **기술적 사실:**
    *   `[CODING_RULES.md](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/CODING_RULES.md)` 및 MISRA-C 규칙에 따라 모든 Unsigned integer 상수에는 명시적인 접미사(`U` 또는 `u`)를 사용해야 합니다.
    *   소스 코드 검토 결과 모든 부호 없는 정수형 상수에 접미사 `U`가 정상 반영되어 있음을 수동 크로스 체크했습니다.

---

## 3. Traceability Verification (추적성 검증)

*   `[traceability_matrix_initial.md](file:///wsl.localhost/Ubuntu-24.04/home/djnoh/repos/rasta-protocol/sil4/docs/traceability/traceability_matrix_initial.md)`를 바탕으로 요구사항-설계-코드-테스트 및 리뷰 기록 간의 연계 관계가 정상적으로 추적되고 있음을 확인했습니다.

---

## 4. Verification Run Result (검증 실행 결과)

*   검증 스크립트 실행을 통해 Configure, Build, 13개 테스트 통과, Cppcheck 정적 분석 경고 0건 상태를 실증했습니다.

## 5. 결론 및 향후 과제

*   전반적인 코드베이스는 드래프트 문서의 스펙을 높은 수준으로 정합 반영하고 있으나, 비동기 멀티태스킹 환경을 대비한 **공개 API의 임계 영역 가드 보호 부재(Finding E)** 및 **세션 리셋 시 타이머 비활성화 누락(Finding F)**과 같은 설계상의 심각한 보안 리스크가 소스 코드에서 추가 발굴되었습니다.
*   인증 가능한 무결성 획득을 위해 본 보고서의 Finding E와 Finding F에 대한 조속한 수정 보완 조치를 권고합니다.

---

## 6. 2026-06-11 조치 결과 최종 검증 및 종결 (V&V Follow-Up Audit)

본 V&V 팀은 개발팀의 조치 내용에 대해 소스 코드, 설계/테스트 문서 검증 및 로컬 빌드/테스트/정적분석 수행을 통하여 다음과 같이 최종 검증을 완료하고 관련 지적 사항들을 종결 처리합니다.

### 6.1 Finding A: 테스트 사양서(TS-002) 내 테스트 케이스 ID 중복 오류
*   **검증 결과:** `[COMPLIANT / CLOSED]`
*   **기술적 사실 및 근거:** `connection_state_machine_test_spec_draft.md` (TS-002) 파일에서 기존에 중복 매핑되어 있던 `TC-SM-012` ID가 단일화되었으며, `SHUTDOWN` 상태의 이벤트 무시 검증 케이스가 `TC-SM-013`으로 재할당되고 순차 재정렬(`TC-SM-001..019`)이 정상 완료된 것을 확인했습니다.

### 6.2 Finding F: 세션 리셋(`rsrx_session_reset`) 시 실행 중인 타이머의 명시적 비활성화(Stop) 누락 우려
*   **검증 결과:** `[COMPLIANT / CLOSED]`
*   **기술적 사실 및 근거:** `rsrx_api.c` 내 `rsrx_session_reset` 함수 시작부에서 `eCancelSessionRuntimeTimers(pxSession)`을 통해 런타임 타이머(`RSRX_TIMER_ID_SUPERVISION`, `RSRX_TIMER_ID_RETRANSMISSION`)에 `RSRX_TIMER_COMMAND_CANCEL` 명령을 명시적으로 발행하는 가드 로직이 소스코드 레벨에서 올바르게 동작함을 확인했습니다.

### 6.3 Finding E: 공개 API 계층의 비동기 호출 간 임계 영역(Critical Section) 보호 부재 (잠재적 Data Race 위협)
*   **검증 결과:** `[COMPLIANT / CLOSED]`
*   **기술적 사실 및 근거:** `rsrx_api.c` 내부의 공개 API 및 비동기 수신 이벤트 핸들러(`eProcessSessionEvent`, `rsrx_session_send_application_data`, `rsrx_session_resolve_inbound_event` 등) 진입 시 플랫폼 어댑터의 크리티컬 섹션 포트(`eEnterSessionCriticalSection`/`eExitSessionCriticalSection`)를 호출하도록 방어 설계가 적용되었습니다. 이를 통해 SafeRTOS 등의 멀티태스킹/ISR 환경에서 세션 공유 컨텍스트 데이터 접근 간의 Data Race 위협이 완벽하게 가드됨을 검증했습니다.

### 6.4 검증 프로그램 빌드 및 실행 결과
*   **검증 결과:** `[COMPLIANT]`
*   **기술적 사실 및 근거:** 로컬 환경에서 `run_ci_verification.sh`를 활용한 완전 순차 검증(`Configure -> Build -> Test -> Cppcheck`)을 수행한 결과, 총 13개 단위/통합 테스트가 모두 통과되었으며 컴파일러 Warning 및 Cppcheck 정적 분석 Warning 모두 **0건(Zero Warning)**을 기록하여 안전성 기준을 만족했습니다.

---

## 7. 신규 안전/규격 결함 발굴 및 조치 권고 (New Audit Findings)

독립 V&V 검증 과정에서 소스 코드 분석을 통해 보안 및 데이터 무결성 측면에서 실질적으로 위협이 될 수 있는 잠재 결함 1건을 추가 식별하여 제기합니다.

### 7.1 [Finding G] Confirmed Timestamp에 대한 과거 시간 윈도우(Past Boundary) 검증 누락 (안전 무결성 취약점)
*   **기술적 사실:**
    *   `[rsrx_codec.c](file:///home/djnoh/repos/rasta-protocol/sil4/src/rsrx_codec.c)` 내의 `rsrx_codec_validate_rasta_sr_timestamp_admission` 함수(Line 897-950)는 수신한 RaSTA SR 패킷의 타임스탬프와 확인 타임스탬프(`uConfirmedTimestamp`)에 대해 시간 윈도우 승인 검사를 수행합니다.
    *   현재 소스 코드(Line 944-947)에는 `uConfirmedTimestamp`가 미래 경계(`uFutureBoundary`)를 초과하는지 여부만 검사하고 있습니다:
        ```c
        if(pxPacket->uConfirmedTimestamp > uFutureBoundary)
        {
            return RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE;
        }
        ```
    *   그러나, `uConfirmedTimestamp`가 허용된 과거 경계(`uPastBoundary`)보다 오래되었는지 여부(`uConfirmedTimestamp < uPastBoundary`)를 검사하는 가드 로직이 **완전히 누락**되어 있습니다.
*   **V&V 평가 및 권고:**
    *   확인 타임스탬프(`uConfirmedTimestamp`)에 대한 과거 시간 윈도우 검증이 누락될 경우, 상대방이 고의적으로 혹은 시간 동기화 에러로 인해 유효 기간이 지난 매우 오래된(Stale) 타임스탬프를 실어 보내더라도 이를 감지하지 못하고 수용하게 됩니다. 이는 비동기 시간 오작동 혹은 Replay Attack 공격 경로에 대한 방어 취약점으로 작용할 수 있습니다.
    *   따라서, `rsrx_codec_validate_rasta_sr_timestamp_admission` 함수 하단에 `uConfirmedTimestamp < uPastBoundary` 여부를 검사하여 `RSRX_CODEC_STATUS_TIMESTAMP_STALE`을 반환하는 안전 가드 코드를 추가할 것을 강력 권고합니다.

---

## 8. 2026-06-12 조치 결과 최종 검증 및 종결 (V&V Follow-Up Audit)

본 V&V 팀은 개발팀이 2026-06-11에 반영한 Finding G의 조치 완료 결과에 대해 소스 코드, 단위 테스트 및 로컬 빌드 검증을 수행하여 다음과 같이 최종 종결 처리합니다.

### 8.1 Finding G: Confirmed Timestamp에 대한 과거 시간 윈도우(Past Boundary) 검증 누락
*   **검증 결과:** `[COMPLIANT / CLOSED]`
*   **기술적 사실 및 근거:** 
    *   **소스 코드 반영:** `rsrx_codec.c` 내의 `rsrx_codec_validate_rasta_sr_timestamp_admission` 함수에 `pxPacket->uConfirmedTimestamp < uPastBoundary` 여부를 검증하고 `RSRX_CODEC_STATUS_TIMESTAMP_STALE`을 반환하는 가드 로직이 정상 반영되었음을 확인했습니다.
    *   **테스트 커버리지 확보:** `test_rsrx_codec.c` (`vTestRastaSrTimestampAdmissionPolicy`) 단위 테스트 코드 상에 stale confirmed timestamp가 유입되었을 때 정상적으로 거부 및 처리되는 시나리오(`rasta sr stale confirmed timestamp rejected`)가 구현 및 통합되어 검증을 통과했습니다.
    *   **문서 및 로드맵 추적성:** `TS-008` (`TC-CODEC-043`) 사양서의 예상 결과 기술 사항이 수정 완료되었으며, `sil4/docs/roadmap_status.md` 및 `OFFICIAL_RESPONSE_TO_VV_REPORTS_2026-04-29.md`에 형상 통제 이력(`RV-477`)이 정합 투영되었음을 확인했습니다.
    *   **종합 검증 빌드 결과:** 픽스가 반영된 상태에서 `run_ci_verification.sh`를 재구동한 결과, 총 13개 단위/통합 테스트 통과 및 컴파일러/정적분석 경고 0건(Zero Warning) 상태를 안정적으로 유지함을 실증하였습니다.
