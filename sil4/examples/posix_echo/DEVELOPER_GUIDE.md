# Developer Guide: Using the SIL4 RASTA-PROTOCOL Core

이 문서는 `sil4/examples/posix_echo` 예제를 기준으로, 개발자가 우리 재구현 RASTA-PROTOCOL 코어를 자기 제품 코드에 어떻게 붙여야 하는지 설명한다.

핵심은 예제 전체를 복사하는 것이 아니라 아래 경계를 유지하는 것이다.

- application은 `rsrx_session_*` public API만 사용한다.
- transport, timer, diagnostics는 port interface 뒤에 둔다.
- codec/profile 선택은 `rsrx_session_config_t`에서 명시한다.
- runtime loop는 receive, timer expiry, application send를 반복적으로 구동한다.
- ISR이나 driver callback에서 session API를 직접 호출하지 않는다.

## 1. 전체 구조

```text
application logic
        |
        v
rsrx_session_* public API
        |
        v
transport supervisor + protocol context + codec
        |
        +---- rsrx_transport_port_t  ---> network/driver binding
        +---- rsrx_platform_port_table_t
        |        +---- clock binding
        |        +---- timer binding
        |        +---- diagnostics binding
        |
        +---- application callback
```

예제에서 이 구조는 다음 파일에 대응된다.

| 역할 | 예제 파일/함수 |
| --- | --- |
| application entry point | `main()` |
| session config 조립 | `vBuildSessionConfig()` |
| transport binding | `posix_transport_init()` |
| platform binding | `posix_platform_init()` |
| receive poll | `vPollSupervisorReceive()` |
| timer expiry 재주입 | `vProcessTimerExpiries()` |
| application send | `vProcessClientSend()` |
| application receive callback | `on_application_data()` |

## 2. Lifecycle Summary

제품 코드에서 유지해야 하는 상위 lifecycle은 아래 순서다.

```text
bind ports -> build config -> init -> start -> connect(client)
    -> receive/timer/send loop -> stop condition -> disconnect(if established) -> cleanup
```

| Phase | Who Calls It | Required API / Function | What Must Be Ready | Expected Result |
| --- | --- | --- | --- | --- |
| transport/platform binding | application startup | `posix_transport_init()` equivalent, `posix_platform_init()` equivalent | driver/socket/link state, monotonic clock, timer command storage, diagnostics sink | `rsrx_transport_port_t` and `rsrx_platform_port_table_t` are fully populated |
| config assembly | application startup | `vBuildSessionConfig()` equivalent | transport port, platform ports, codec port, encode buffer, callbacks, channel topology | `rsrx_session_config_t` represents the full integration contract |
| session init | application startup | `rsrx_session_init()` | complete config, valid buffers, selected channel policy | session object is initialized or startup fails early |
| session start | application startup | `rsrx_session_start()` | initialized session | timers/diagnostics/state baseline are started |
| client connect | client role only | `rsrx_session_connect()` | started session, remote endpoint reachable | handshake begins; expected state progression reaches `ESTABLISHED` |
| receive poll | runtime loop/task | `rsrx_transport_supervisor_poll_receive()` or bounded pump equivalent | transport receive callback can return frames or unavailable status | inbound frames become session events and application callbacks |
| timer expiry | runtime loop/task | `rsrx_session_process_timer_expiry()` | timer command deadlines are tracked against monotonic time | supervision, retransmission, and diagnostics timers remain active |
| application send | runtime loop/application trigger | `rsrx_session_send_application_data()` | session is `RSRX_STATE_ESTABLISHED`; payload buffer is valid for the call | encoded data is queued/sent through the selected channel |
| application receive | callback | `pfApplicationData` callback | callback context and payload ownership policy are defined | application consumes or copies payload |
| graceful disconnect | shutdown path | `rsrx_session_disconnect()` when state is `RSRX_STATE_ESTABLISHED` | no other task is concurrently driving the same session | disconnect frame is requested before local resources are released |
| stop/cleanup | shutdown path | application-owned cleanup, transport cleanup | session API calls have stopped or are serialized | sockets/driver handles/timers are released by the port layer |

The example logs that prove the lifecycle is moving:

| Lifecycle Point | Expected Example Log |
| --- | --- |
| start | `Starting server ...` or `Starting client ...` |
| established | `State change: 2 -> 3` |
| client send | `TX[1]: ...` |
| server receive | `RX[1]: ...` in server log |
| echo send | `Echo send status=0` |
| client receive | `RX[1]: ...` in client log |
| failover | `Channel report: active=secondary ... failover=1` |
| graceful disconnect | `Graceful disconnect status=0` when stopping from `ESTABLISHED` |
| cleanup | `Stopped role=... tx=... rx=... final_state=...` |

If the lifecycle stops at a specific row, use [TROUBLESHOOTING.md](./TROUBLESHOOTING.md) from that row downward instead of debugging the whole stack at once.

## 3. 제품 코드에 붙이는 최소 순서

### Step 1: transport port를 만든다

`rsrx_transport_port_t`는 코어가 실제 link를 모르고도 send/receive/query를 수행하게 해주는 경계다.

제품 코드에서는 아래 3개 callback을 실제 driver 또는 network stack에 연결한다.

```c
rsrx_transport_port_t xTransportPort;

xTransportPort.pvContext = &xMyTransportContext;
xTransportPort.pfSend = my_transport_send;
xTransportPort.pfReceive = my_transport_receive;
xTransportPort.pfQueryChannel = my_transport_query_channel;
```

예제에서는 [`posix_transport_init()`](./posix_transport.c)가 이 일을 한다.

구현할 때 지켜야 할 점:

- `pfSend`는 payload pointer, length, channel ID를 받아 실제 link로 전송한다.
- `pfReceive`는 수신 frame을 `rsrx_transport_frame_t`로 채운다.
- `pfQueryChannel`은 channel up/down 상태를 반환한다.
- receive buffer의 수명은 supervisor가 처리하는 동안 유효해야 한다.
- 사용할 수 없는 channel은 `CHANNEL_DOWN` 또는 `UNAVAILABLE`처럼 구분 가능한 status로 반환한다.

### Step 2: platform port를 만든다

`rsrx_platform_port_table_t`는 시간, 타이머, diagnostics를 실제 플랫폼에 연결한다.

```c
rsrx_platform_port_table_t xPlatformPorts;

xPlatformPorts.xClock.pvContext = &xClockContext;
xPlatformPorts.xClock.pfNow = my_clock_now_ns;

xPlatformPorts.xTimer.pvContext = &xTimerContext;
xPlatformPorts.xTimer.pfCommand = my_timer_command;

xPlatformPorts.xDiagnostics.pvContext = &xDiagContext;
xPlatformPorts.xDiagnostics.pfWrite = my_diagnostic_write;
```

예제에서는 [`posix_platform_init()`](./posix_platform.c)가 이 일을 한다.

구현할 때 지켜야 할 점:

- `pfNow`는 단조 증가하는 ns 단위 시간을 반환해야 한다.
- timer command는 supervision, retransmission, diagnostic flush timer를 구분해서 처리한다.
- timer가 만료되면 application loop 또는 communication task가 `rsrx_session_process_timer_expiry()`를 호출해야 한다.
- diagnostics는 최소한 상태 전이, status, reason, event counter를 추적 가능하게 남긴다.

### Step 3: application callback을 준비한다

상위 application이 데이터를 받는 지점은 callback이다.

```c
static void my_application_data(
    void * pvContext,
    const rsrx_orchestrator_report_t * pxReport,
    const rsrx_application_data_indication_t * pxIndication)
{
    (void)pvContext;
    (void)pxReport;

    /* pxIndication->puPayload / xPayloadLength를 application으로 넘긴다. */
}
```

예제에서는 [`on_application_data()`](./main.c)가 이 역할을 한다.

실제 제품에서는 callback 안에서 무거운 처리를 바로 하지 않는 편이 좋다. 필요한 경우 payload를 application queue로 넘기고, 별도 task에서 처리한다.

### Step 4: session config를 한 곳에서 조립한다

`rsrx_session_config_t`는 application이 코어에 주는 전체 계약이다.

```c
rsrx_session_config_t xConfig;

(void)memset(&xConfig, 0, sizeof(xConfig));
xConfig.xTransportPort = xTransportPort;
xConfig.xCodecPort = *rsrx_codec_get_default_port();
xConfig.xPlatformPorts = xPlatformPorts;
xConfig.puFramePayload = auEncodeBuffer;
xConfig.xFramePayloadLength = sizeof(auEncodeBuffer);
xConfig.pfApplicationData = my_application_data;
xConfig.pvApplicationDataContext = &xApplicationContext;
```

예제에서는 [`vBuildSessionConfig()`](./main.c)가 이 역할을 한다.

구현할 때 확인할 항목:

- `xTransportPort`가 모두 채워졌는가
- `xPlatformPorts`의 clock/timer/diagnostics가 모두 채워졌는가
- `xCodecPort`가 선택되었는가
- encode buffer pointer와 length가 유효한가
- application callback과 context가 설정되었는가
- redundancy 사용 시 channel-manager topology가 설정되었는가

### Step 5: session을 시작한다

server와 client 모두 session을 init/start한다. client 역할은 connect도 호출한다.

```c
rsrx_session_t xSession;
const rsrx_orchestrator_report_t * pxReport = 0;

if(rsrx_session_init(&xSession, &xConfig) != RSRX_STATUS_OK)
{
    /* configuration or platform binding error */
}

if(rsrx_session_start(&xSession, &pxReport) != RSRX_STATUS_OK)
{
    /* startup failure */
}

if(is_client)
{
    (void)rsrx_session_connect(&xSession, &pxReport);
}
```

예제에서는 `main()`에서 이 순서를 그대로 보여준다.

### Step 6: runtime loop를 돌린다

코어는 background thread를 만들지 않는다. application이 receive와 timer expiry를 계속 구동해야 한다.

```text
while(running)
{
    receive 가능한 frame이 있으면 supervisor pump 호출
    timer deadline이 지났으면 session timer expiry 호출
    ESTABLISHED 상태이면 필요한 application data 송신
}
```

예제에서는 다음 함수들이 이 역할을 나눠 맡는다.

- `vPollSupervisorReceive()`
- `vProcessTimerExpiries()`
- `vProcessClientSend()`

제품 코드에서는 POSIX `select()` 대신 RTOS task loop, event queue, semaphore wait, driver poll 등으로 치환하면 된다.

## 4. Single Channel 구성

가장 작은 구성은 primary channel 하나만 사용하는 것이다.

```c
xConfig.eDefaultChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
xConfig.xChannelManagerConfig.eMode = RSRX_REDUNDANCY_MODE_SINGLE;
xConfig.xChannelManagerConfig.uChannelCount = 1U;
xConfig.xChannelManagerConfig.axChannels[0].eChannelId =
    RSRX_TRANSPORT_CHANNEL_PRIMARY;
xConfig.xChannelManagerConfig.axChannels[0].uIsAvailable = 1U;
```

예제 기본 실행이 이 구성이다.

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build run-server
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build run-client
```

## 5. Active-Standby Redundancy 구성

primary/secondary link를 쓰려면 channel manager를 active-standby로 구성한다.

```c
xConfig.eDefaultChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
xConfig.xChannelManagerConfig.eMode = RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY;
xConfig.xChannelManagerConfig.uChannelCount = 2U;
xConfig.xChannelManagerConfig.axChannels[0].eChannelId =
    RSRX_TRANSPORT_CHANNEL_PRIMARY;
xConfig.xChannelManagerConfig.axChannels[0].uPriority = 0U;
xConfig.xChannelManagerConfig.axChannels[0].uIsAvailable = 1U;
xConfig.xChannelManagerConfig.axChannels[1].eChannelId =
    RSRX_TRANSPORT_CHANNEL_SECONDARY;
xConfig.xChannelManagerConfig.axChannels[1].uPriority = 1U;
xConfig.xChannelManagerConfig.axChannels[1].uIsAvailable = 1U;
```

예제에서는 `--redundant 1` 옵션이 이 구성을 켠다.

failover를 구현할 때 중요한 점:

- link down/up 이벤트는 transport/channel layer에서 감지한다.
- 감지된 이벤트는 supervisor에 channel event로 전달한다.
- application은 active channel을 직접 선택하지 않는다.
- active channel 변경은 supervisor/channel-manager report로 관찰한다.

## 6. Application Data 송수신

송신은 `ESTABLISHED` 상태에서 `rsrx_session_send_application_data()`로 수행한다.

```c
if(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED)
{
    (void)rsrx_session_send_application_data(
        &xSession,
        puPayload,
        xPayloadLength);
}
```

수신은 application callback으로 들어온다.

주의할 점:

- callback payload pointer의 수명을 application이 임의로 길게 잡으면 안 된다.
- callback 밖에서 사용할 데이터는 복사하거나 명확한 ownership 정책을 둔다.
- 송신 실패 status는 무시하지 말고 diagnostics 또는 telemetry에 남긴다.

## 7. Timer 처리 모델

session은 timer command를 platform port로 요청한다. application 또는 port layer는 그 deadline을 보관한다.

deadline이 지나면 다음 API를 호출한다.

```c
rsrx_session_process_timer_expiry(
    &xSession,
    RSRX_TIMER_EXPIRY_SUPERVISION,
    &pxReport);
```

예제는 단순화를 위해 POSIX loop에서 deadline을 직접 비교한다. RTOS에서는 software timer callback이 직접 session을 호출하기보다 communication task에 event를 보내고, task context에서 expiry를 처리하는 구조가 안전하다.

## 8. 종료와 cleanup 처리

정상 종료에서는 transport resource를 바로 닫기 전에 session 상태를 확인한다.

```c
if(rsrx_session_get_state(&xSession) == RSRX_STATE_ESTABLISHED)
{
    (void)rsrx_session_disconnect(&xSession, &pxReport);
}
```

이 순서는 remote peer에게 disconnect intent를 보낼 기회를 제공한다. 이미 `SAFE_DISCONNECT`에 들어간 경우에는 추가 disconnect를 보내지 말고 local cleanup으로 진행한다.

제품 코드에서는 다음 원칙을 유지한다.

- session API 호출은 하나의 communication task 또는 동일한 critical-section 정책 아래에서 직렬화한다.
- disconnect 요청 후 target driver/socket close는 port layer가 소유한다.
- disconnect 전송 성공은 graceful 종료의 보조 evidence일 뿐이며, 상대 peer 수신까지 보장하지 않는다.
- 강제 종료, watchdog reset, 전원 차단 같은 경로는 별도 safety shutdown evidence로 다룬다.

## 9. Diagnostics와 오류 처리

제품 코드에서 최소한 남겨야 할 정보:

- 상태 전이: previous state, next state
- API 또는 supervisor status
- disconnect/safe-disconnect reason
- channel active/available/switch count
- timer command 실패
- send/receive 실패 status

예제에서는 stdout 로그로 보여주지만, 실제 제품에서는 event recorder, ring buffer, NVM snapshot, field diagnostic channel 등 프로젝트 표준 경로로 연결한다.

## 10. 개발자가 바꾸는 것과 유지하는 것

바꾸는 것:

- UDP socket 대신 실제 driver/RTOS network stack
- POSIX clock/timer 대신 target monotonic clock/timer service
- stdout diagnostics 대신 프로젝트 diagnostics
- `select()` loop 대신 communication task/event loop
- demo failover injection 대신 실제 link monitor

유지하는 것:

- `rsrx_session_config_t`를 한 곳에서 조립하는 구조
- transport/platform/codec을 명시적으로 주입하는 구조
- `init -> start -> connect -> receive/timer/send loop` 순서
- application data는 public API와 callback으로만 교환하는 구조
- ISR/driver callback과 session core를 직접 결합하지 않는 구조

## 11. Production Usage Guardrails

제품 코드로 옮길 때 아래 패턴은 금지하거나 명시적 review 없이는 사용하지 않는다.

| Pattern | Rule | Reason |
| --- | --- | --- |
| ISR에서 session API 직접 호출 | 금지 | session/state/timer/diagnostics 처리는 task context에서 순서가 보장되어야 한다 |
| RTOS timer callback에서 session API 직접 호출 | 금지 | timer callback priority와 session 처리 순서가 섞이면 재현 어려운 race가 생긴다 |
| application callback에서 긴 blocking 작업 수행 | 금지 | receive pump와 timer expiry 처리가 지연되어 supervision/retransmission 동작이 왜곡된다 |
| callback payload pointer 장기 보관 | 금지 | payload buffer 소유권은 port/supervisor 처리 범위 안에 있으므로 장기 보관 시 stale pointer가 된다 |
| active channel을 application이 캐시 | 금지 | failover 후에도 application이 primary로 송신하는 오류가 생긴다 |
| `pfReceive`에서 수신 없음과 수신 오류를 같은 status로 반환 | 금지 | supervisor가 retry 가능한 상태와 실제 오류를 구분할 수 없다 |
| unbounded driver queue 사용 | review 필요 | bounded behavior와 backpressure evidence를 해칠 수 있다 |
| hidden global buffer ownership | review 필요 | callback, send path, driver DMA 사이 ownership이 불명확해진다 |
| diagnostics callback 비워두기 | 금지 | 상태 전이, reason, timer failure, channel switch를 사후 추적할 수 없다 |
| monotonic clock 단위 임의 변환 | 금지 | ns 단위 계약이 깨지면 supervision/retransmission timing이 틀어진다 |

권장 구조:

- ISR/driver callback은 frame arrival 또는 timer event를 queue/semaphore/notification으로 communication task에 넘긴다.
- communication task만 `rsrx_session_*` API와 `rsrx_transport_supervisor_*` API를 호출한다.
- application callback은 payload를 복사하거나 명확한 ownership queue로 넘기고 빠르게 반환한다.
- send failure, receive failure, timer failure, channel switch는 diagnostics 또는 telemetry에 남긴다.
- driver queue, application queue, deferred send queue는 bounded capacity와 overflow policy를 문서화한다.

이 예제는 POSIX 단일 프로세스라 단순하게 보이지만, production target에서는 위 guardrail을 지키는 것이 더 중요하다.

## 12. 첫 통합 체크리스트

1. `rsrx_session_init()`이 `RSRX_STATUS_OK`를 반환한다.
2. `rsrx_session_start()`가 성공한다.
3. client에서 `rsrx_session_connect()`를 호출한다.
4. client/server가 `ESTABLISHED` 상태에 도달한다.
5. client `rsrx_session_send_application_data()`가 성공한다.
6. server application callback이 payload를 받는다.
7. server echo 또는 application response가 client callback으로 돌아온다.
8. supervision/retransmission timer expiry가 task context에서 처리된다.
9. channel down 상황에서 status/report가 관찰된다.
10. redundancy 사용 시 secondary channel로 전환 후 traffic이 유지된다.

## 13. SafeRTOS / AM263Px로 옮길 때

SafeRTOS / AM263Px에서는 이 문서의 구조를 그대로 유지하고, POSIX 구현만 치환한다.

- `main()` loop -> communication task
- `posix_transport_init()` -> AM263Px Ethernet/CPSW/Enet/프로젝트 link driver binding
- `posix_platform_init()` -> SafeRTOS timer, monotonic clock, diagnostics binding
- demo channel toggle -> 실제 link monitor event
- stdout log -> target diagnostics/event recorder

세부 포팅 메모는 [PORTING_SAFERTOS_AM263PX.md](./PORTING_SAFERTOS_AM263PX.md)를 참고한다.

## 14. Smoke로 확인할 것

기본 echo:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build smoke
```

redundancy/failover:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build failover-smoke
```

이 두 smoke가 통과하면 예제 수준에서는 아래가 확인된다.

- session init/start/connect 흐름
- application data send/receive callback
- timer expiry 재주입
- transport supervisor receive pump
- active-standby channel switch
- failover 후 echo traffic 유지
