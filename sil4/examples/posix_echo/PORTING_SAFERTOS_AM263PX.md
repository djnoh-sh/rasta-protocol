# SafeRTOS / AM263Px Porting Notes

이 문서는 `sil4/examples/posix_echo` 예제를 기준으로, 우리 재구현 RASTA-PROTOCOL 코드를 `SafeRTOS + AM263Px` 환경에 포팅할 때 무엇을 바꿔야 하는지 정리한 메모다.

핵심은 "POSIX API를 없애고도 같은 port contract를 유지하는 것"이다.

## 1. Task 구조

POSIX 예제의 `main()` loop는 SafeRTOS에서는 보통 전용 communication task 하나로 옮긴다.

- 예제의 `select() + receive + timer expiry + periodic send` 루프
  - SafeRTOS에서는 `vRastaCommTask()` 같은 task로 옮긴다
- application callback가 상위 제어 task와 직접 섞이지 않게
  - queue나 mailbox를 통해 application layer로 넘기는 편이 안전하다
- blocking time을 길게 잡기보다
  - 짧은 주기 poll + timer expiry 처리로 유지하는 편이 예제 구조와 가깝다

즉, POSIX의 "프로세스 하나"는 SafeRTOS에서는 "통신 전용 task 하나 + 필요하면 application handoff queue"로 치환하면 된다.

## 2. Transport Binding

이 예제의 [`posix_transport_init()`](./posix_transport.c)는 AM263Px에서는 보통 아래 둘 중 하나로 치환된다.

1. Ethernet/UDP stack 사용
   - MCU+ SDK 기반 Ethernet stack 위에서 UDP send/receive wrapper를 작성
   - wrapper 내부에서 `pfSend`, `pfReceive`, `pfQueryChannel`를 채운다

2. 더 하위 driver 직접 사용
   - CPSW/Enet driver 기반 raw frame path 또는 프로젝트 전용 링크 계층 사용
   - 이 경우에도 외부로는 `rsrx_transport_port_t` 계약만 유지한다

중요한 점:

- `pfReceive`는 ISR에서 직접 session을 호출하지 말고
  - driver/stack 수신을 task context에서 꺼내 `rsrx_transport_supervisor_pump_receive()`로 넘기는 편이 안전하다
- redundancy를 쓸 경우
  - AM263Px의 실제 포트/링크 두 개를 `PRIMARY`, `SECONDARY`로 매핑하는 테이블이 필요하다
- link-down/link-up 신호가 있으면
  - 예제의 `CHANNEL_DOWN`, `CHANNEL_UP` 처리와 같은 방향으로 supervisor에 연결할 수 있다

## 3. Timer Binding

이 예제의 [`posix_platform_init()`](./posix_platform.c)는 SafeRTOS/AM263Px에서 가장 먼저 바뀌는 부분이다.

- `xClock.pfNow`
  - AM263Px의 monotonic time source로 치환
  - 핵심은 반드시 "단조 증가"하고 ns 해상도로 환산 가능해야 한다는 점이다
- `xTimer.pfCommand`
  - supervision / retransmission / diagnostic flush timer를 실제 RTOS timer 또는 hardware timer abstraction에 연결
- `xDiagnostics.pfWrite`
  - UART log, shared memory log, event recorder 등 프로젝트 표준 diagnostics 경로로 연결

실무적으로는 보통:

- time base
  - SoC free-running counter 또는 RTOS tick + 보정
- timer command
  - SafeRTOS software timer 또는 dedicated timer service task
- diagnostics
  - low-priority logger task 또는 lock-free trace buffer

## 4. Interrupt / Task 경계

AM263Px에서는 ISR와 task 경계를 명확히 하는 것이 중요하다.

- ISR에서 바로 session API를 호출하지 않는다
- ISR에서는
  - 수신 사실 기록
  - semaphore/notification give
  - queue push
  정도만 수행한다
- 실제 `pfReceive`, `process_timer_expiry`, application callback dispatch는 task context에서 수행한다

이 원칙은 SIL4 재구현의 추적성과 결정성을 유지하는 데도 유리하다.

## 5. Memory / Buffer 정책

POSIX 예제는 단순성을 위해 stack/static buffer를 섞어 쓰지만, SafeRTOS/AM263Px 포팅에서는 buffer 소유권을 더 엄격히 잡는 편이 좋다.

- encode buffer
  - `rsrx_session_config_t.puFramePayload`에 연결하는 송신 버퍼는 수명과 동시성 규칙을 명확히 한다
- receive buffer
  - DMA/driver buffer를 직접 노출할지, task local buffer로 복사할지 정책을 먼저 정한다
- callback payload
  - 상위 task로 넘길 때는 deep copy인지 zero-copy인지 명시한다

특히 safety review 관점에서는 "누가 언제 버퍼를 소유하는가"를 문서로 남겨두는 편이 좋다.

## 6. Redundancy Mapping

예제의 `PRIMARY`, `SECONDARY`는 AM263Px 포팅에서는 보통 다음 중 하나에 대응된다.

- 물리 Ethernet port A / port B
- 같은 MAC의 이중 링크 경로
- 서로 다른 외부 모뎀/통신 모듈 채널

중요한 것은 "driver 구조"가 아니라 "channel identity를 supervisor가 안정적으로 구분할 수 있는가"다.

따라서 포팅 시에는 먼저 아래를 정하면 된다.

1. 어떤 링크를 `PRIMARY`로 볼 것인가
2. 어떤 링크를 `SECONDARY`로 볼 것인가
3. 각 링크의 up/down 신호를 어디서 받을 것인가
4. link recovery 후 preferred recovery holdoff를 사용할 것인가

## 7. SafeRTOS / AM263Px 포팅 체크포인트

포팅 첫 단계에서는 아래만 먼저 통과하면 된다.

1. single-channel
   - `rsrx_session_init()` / `start()` / `connect()`가 동작
   - supervision / retransmission timer expiry가 task context에서 처리
   - application data 1회 송수신 성공

2. diagnostics
   - state transition, reason, timer failure가 로그에 남음

3. redundancy
   - `PRIMARY` down 시 `SECONDARY`로 전환
   - 전환 뒤에도 receive/send가 계속 유지

4. stress
   - receive burst 중에도 timer expiry와 send path가 교착 없이 유지

## 8. 이 예제에서 그대로 참고할 함수

- session config 조립: [`vBuildSessionConfig()`](./main.c)
- transport endpoint 개념: [`vBuildTransportEndpoints()`](./main.c)
- timer expiry 재주입 구조: `vProcessTimerExpiries()`
- periodic send 구조: `vProcessClientSend()`
- failover event 주입 흐름: `vProcessChannelToggle()`

즉, AM263Px 포팅에서는 POSIX API 자체를 따라 하기보다, 위 함수들이 보여주는 "역할 분리"를 그대로 유지하는 것이 더 중요하다.
