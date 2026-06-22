# SIL4 재구현 과제 기능 일치성(Parity) 및 구조적 정밀 평가

본 문서는 기존 `rasta-protocol` 레거시 프로토타입 코드와 현재 `sil4/` 하위에 새로 구현된 SIL4 기준 대상 코드를 **기능의 완전성(Functional Parity)** 및 **구조적 안전성(Structural Integrity)** 관점에서 비교 분석한 결과입니다.

---

## 1. 개요 요약
신규 구현된 코드 베이스는 기존 코드 대비 SIL4 인증을 위한 **격리성, 추적성, 방어적 상태 머신 구조가 압도적으로 우수**합니다. 하지만 현재 P3/P4 단계에서는 통신 상태의 전이 로직 및 이중화 관리에만 초점을 맞추었기 때문에, **실제 RaSTA 프로토콜이 요구하는 데이터 무결성(CRC)과 보안(MAC), 그리고 시간 감시(Timestamp) 기능은 의도적으로 단순화된 뼈대(Skeleton) 상태로 남아있습니다.**

---

## 2. 기능 구현 관점의 누락/미비점 (Feature Parity Gap)

원형 규격(DIN VDE V 0831-200) 및 기존 레거시 코드에 존재하나 신규 코드에는 아직 구현되지 않은 핵심 기능들입니다.

### 2.1 암호화 및 무결성 검증 (MAC & CRC) 부재
* **기존 구현:** `MD4`, `Blake2b` 알고리즘 기반의 메시지 인증 코드(MAC) 생성/검증 및 네트워크 바이트 스트림에 대한 덧붙임 `CRC` 연산 수행 (`rastacrc.c`, `rastablake2.c`, `rastamd4.c`).
* **현재 구현:** `sil4/src/rsrx_codec.c` 내부 어디에도 해시나 CRC 연산 로직이 존재하지 않으며, 바이트 스트림 무결성 훼손에 대한 방어가 빠져 있습니다.

### 2.2 타임스탬프(Timestamp) 기반 시간 감시 부재
* **기존 구현:** 패킷 교환 시 송수신 타임스탬프를 대조하여 메시지의 지연(Staleness)이나 재전송 공격(Replay Attack)을 차단.
* **현재 구현:** `rsrx_decoded_message_t` 구조체 및 관련 모듈 전체에서 `timestamp` 필드 자체가 없으며, 시간 감시 로직이 생략되어 있습니다.

### 2.3 표준 패킷 포맷(PDU) 미준수
* **현재 구현:** `[MsgType][Reason][Reserved][Seq][Confirm][Len][Reserved]` 형태의 매우 단순화된 **16바이트 스켈레톤 커스텀 헤더**를 임시로 사용 중입니다. 네트워크 ID, 안전 코드, 타임스탬프 등을 포함한 복잡한 진짜 RaSTA 헤더 파싱 규칙이 아직 이식되지 않았습니다.

---

## 3. 구조적 평가 및 장점 (Structural Assessment)

기능적 부재에도 불구하고, 새롭게 작성된 시스템의 구조는 완벽에 가깝게 설계되었습니다.

### 3.1 완벽한 격리와 추상화 (Dependency Injection)
* 기존 코드는 파싱, 상태 머신, 네트워크 소켓 송수신이 강하게 결합(Coupling)된 형태였습니다.
* 신규 코드는 `rsrx_codec_port_t`, `rsrx_transport_port_t` 구조체를 통한 인터페이스 포인터 방식을 채택하여 **OS나 네트워크 스택에 구애받지 않고 오직 순수 로직만 떼어내어 100% 모의(Mocking) 테스트가 가능**하게 만들었습니다. 이는 메모리 동적 할당을 금지하는 SIL4 원칙을 지키는 가장 이상적인 구조입니다.

### 3.2 철저한 상태 감시 및 텔레메트리 (Telemetry)
* `rsrx_transport_supervisor.c`를 통해 모든 비정상 프레임 수신 및 송신 실패가 예산(Budget) 단위로 엄격하게 관리되고 있으며, 이 모든 상태 변이와 에러 카운트가 `pxContext->xLastReport`에 기록되어 역추적(Audit)이 가능하게 구조화되어 있습니다.

---

## 4. 향후 보완 제언 (Recommended Next Steps)

다음 단계의 고도화 작업 시 아래의 3가지가 최우선으로 코드에 반영되어야 합니다.

1. **표준 패킷 코덱(Codec) 이식:** 임시 16바이트 헤더를 제거하고, 기존 코드의 `rastacrc.c` 등을 SIL4 코딩 룰(정적 분석 Warning 제로, MISRA-C 준수)에 맞게 리팩토링하여 `rsrx_codec.c`에 연결해야 합니다.
2. **에러 타입 및 진단(Diagnostic) 세분화:** 코덱 파싱 실패 시, `rsrx_codec_status_t` 열거형에 `RSRX_CODEC_STATUS_MAC_MISMATCH`, `RSRX_CODEC_STATUS_STALE_TIMESTAMP` 등의 에러 코드를 추가하여, 수퍼바이저(Supervisor)가 어떤 보안 위협으로 패킷을 버렸는지 상위 계층에 정확히 리포트하도록 해야 합니다.
3. **타임스탬프 기반 확인(Confirm) 고도화:** 로드맵 `NS-001`에 명시된 바와 같이, 타임스탬프 기반으로 Stale Feedback(너무 늦게 도착한 과거의 메시지)을 걸러내는 정교한 Sequence Validation 로직을 Protocol Context 로직 안에 덧붙여야 합니다.
4. **실제 구동을 위한 예제 프로그램(Example Application) 제작:** 현재 방대한 통합 테스트(`TC-INT-*`)는 모두 메모리 상의 가짜 소켓(Mock Transport)과 가짜 시간(Fake Time) 위에서 동작합니다. 신규 코드가 실제 환경(POSIX UDP 소켓, Linux Timer)과 어떻게 결합하여(Binding) 구동되는지 보여주는 레퍼런스 예제 코드(예: `sil4/examples/rsrx_echo_server.c`) 작성이 필수적입니다. 비록 예제 코드 자체가 SIL4 인증 대상은 아니더라도, 상위 어플리케이션 개발자들을 위한 가이드 역할을 위해 반드시 필요합니다.
