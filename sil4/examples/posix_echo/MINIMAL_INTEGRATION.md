# Minimal Integration Guide

이 문서는 우리 재구현 RASTA-PROTOCOL 코어를 제품 코드에 붙일 때 필요한 최소 skeleton을 정리한다.

자세한 설명은 [DEVELOPER_GUIDE.md](./DEVELOPER_GUIDE.md)를 보고, 이 문서는 “무엇을 어떤 순서로 구현해야 하는가”를 빠르게 확인할 때 사용한다.

## 1. 필요한 구성 요소

제품 쪽에서 준비해야 하는 것은 네 가지다.

| 항목 | 제품 코드에서 해야 할 일 | 예제 위치 |
| --- | --- | --- |
| transport binding | send/receive/channel-query callback 구현 | `posix_transport.c` |
| platform binding | monotonic clock/timer/diagnostics callback 구현 | `posix_platform.c` |
| session config | transport/platform/codec/callback/channel topology 조립 | `vBuildSessionConfig()` |
| runtime loop | receive pump, timer expiry, application send 반복 실행 | `main()` loop |

## 2. 최소 파일 구조 예시

실제 제품에서는 아래처럼 나누는 것이 가장 단순하다.

```text
my_rasta_port.h
my_rasta_transport.c
my_rasta_platform.c
my_rasta_session.c
my_rasta_task.c
```

각 파일의 책임:

- `my_rasta_transport.c`: driver 또는 network stack을 `rsrx_transport_port_t`로 감싼다.
- `my_rasta_platform.c`: clock, timer, diagnostics를 `rsrx_platform_port_table_t`로 감싼다.
- `my_rasta_session.c`: `rsrx_session_config_t`를 조립하고 session을 시작한다.
- `my_rasta_task.c`: communication task 또는 event loop를 실행한다.

## 3. Transport skeleton

```c
typedef struct
{
    /* driver handle, socket, DMA queue, link state cache 등 */
    uint32_t uPrimaryUp;
    uint32_t uSecondaryUp;
} my_transport_context_t;

static rsrx_transport_status_t my_send(
    void * pvContext,
    const rsrx_transport_send_request_t * pxRequest)
{
    my_transport_context_t * pxCtx = (my_transport_context_t *)pvContext;

    if((pxCtx == 0) || (pxRequest == 0) || (pxRequest->puPayload == 0))
    {
        return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
    }

    /* pxRequest->eChannelId에 맞는 driver/link로 전송한다. */
    /* driver busy면 UNAVAILABLE, link down이면 CHANNEL_DOWN을 반환한다. */

    return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t my_receive(
    void * pvContext,
    rsrx_transport_frame_t * pxFrame)
{
    my_transport_context_t * pxCtx = (my_transport_context_t *)pvContext;

    if((pxCtx == 0) || (pxFrame == 0))
    {
        return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
    }

    /* pxFrame->eChannelId에 맞는 수신 큐/driver에서 frame을 꺼낸다. */
    /* 수신 없음은 UNAVAILABLE로 반환한다. */
    /* 수신 성공 시 puPayload, xPayloadLength, eEventType을 채운다. */

    return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t my_query_channel(
    void * pvContext,
    rsrx_transport_channel_state_t * pxState)
{
    my_transport_context_t * pxCtx = (my_transport_context_t *)pvContext;

    if((pxCtx == 0) || (pxState == 0))
    {
        return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
    }

    if(pxState->eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY)
    {
        pxState->uIsAvailable = pxCtx->uPrimaryUp;
    }
    else if(pxState->eChannelId == RSRX_TRANSPORT_CHANNEL_SECONDARY)
    {
        pxState->uIsAvailable = pxCtx->uSecondaryUp;
    }
    else
    {
        return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
    }

    return RSRX_TRANSPORT_STATUS_OK;
}

static void my_transport_init(
    my_transport_context_t * pxCtx,
    rsrx_transport_port_t * pxPort)
{
    pxPort->pvContext = pxCtx;
    pxPort->pfSend = my_send;
    pxPort->pfReceive = my_receive;
    pxPort->pfQueryChannel = my_query_channel;
}
```

## 4. Platform skeleton

```c
typedef struct
{
    uint32_t uSupervisionActive;
    rsrx_monotonic_time_ns_t uSupervisionDeadline;
    uint32_t uRetransmissionActive;
    rsrx_monotonic_time_ns_t uRetransmissionDeadline;
    uint32_t uDiagnosticFlushActive;
    rsrx_monotonic_time_ns_t uDiagnosticFlushDeadline;
} my_timer_context_t;

static rsrx_platform_status_t my_now(
    void * pvContext,
    rsrx_monotonic_time_ns_t * puNowNs)
{
    (void)pvContext;

    if(puNowNs == 0)
    {
        return RSRX_PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    /* target monotonic counter를 ns 단위로 환산한다. */
    *puNowNs = my_target_monotonic_time_ns();
    return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t my_timer_command(
    void * pvContext,
    const rsrx_timer_command_t * pxCommand)
{
    my_timer_context_t * pxTimer = (my_timer_context_t *)pvContext;

    if((pxTimer == 0) || (pxCommand == 0))
    {
        return RSRX_PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    /* START/RESTART/CANCEL을 timer ID별 deadline/cache로 반영한다. */
    /* RTOS timer를 직접 쓰는 경우에도 expiry 처리는 task context로 넘긴다. */

    return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t my_diagnostic_write(
    void * pvContext,
    const rsrx_diagnostic_record_t * pxRecord)
{
    (void)pvContext;

    if(pxRecord == 0)
    {
        return RSRX_PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    /* project logger, event recorder, ring buffer 등에 기록한다. */
    return RSRX_PLATFORM_STATUS_OK;
}

static void my_platform_init(
    my_timer_context_t * pxTimer,
    rsrx_platform_port_table_t * pxPlatform)
{
    pxPlatform->xClock.pvContext = 0;
    pxPlatform->xClock.pfNow = my_now;
    pxPlatform->xTimer.pvContext = pxTimer;
    pxPlatform->xTimer.pfCommand = my_timer_command;
    pxPlatform->xDiagnostics.pvContext = 0;
    pxPlatform->xDiagnostics.pfWrite = my_diagnostic_write;
}
```

## 5. Session config skeleton

```c
typedef struct
{
    rsrx_session_t xSession;
    rsrx_transport_supervisor_context_t xSupervisor;
    rsrx_transport_port_t xTransportPort;
    rsrx_platform_port_table_t xPlatformPorts;
    my_transport_context_t xTransportContext;
    my_timer_context_t xTimerContext;
    uint8_t auEncodeBuffer[1024];
} my_rasta_context_t;

static void my_application_data(
    void * pvContext,
    const rsrx_orchestrator_report_t * pxReport,
    const rsrx_application_data_indication_t * pxIndication)
{
    (void)pvContext;
    (void)pxReport;

    if(pxIndication == 0)
    {
        return;
    }

    /* payload를 application queue로 넘기거나 즉시 처리한다. */
}

static int my_rasta_init(
    my_rasta_context_t * pxCtx,
    uint32_t uUseRedundancy)
{
    rsrx_session_config_t xConfig;
    const rsrx_orchestrator_report_t * pxReport = 0;

    my_transport_init(&pxCtx->xTransportContext, &pxCtx->xTransportPort);
    my_platform_init(&pxCtx->xTimerContext, &pxCtx->xPlatformPorts);

    (void)memset(&xConfig, 0, sizeof(xConfig));
    xConfig.xTransportPort = pxCtx->xTransportPort;
    xConfig.xPlatformPorts = pxCtx->xPlatformPorts;
    xConfig.xCodecPort = *rsrx_codec_get_default_port();
    xConfig.puFramePayload = pxCtx->auEncodeBuffer;
    xConfig.xFramePayloadLength = sizeof(pxCtx->auEncodeBuffer);
    xConfig.pfApplicationData = my_application_data;
    xConfig.pvApplicationDataContext = pxCtx;
    xConfig.eDefaultChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;

    xConfig.xChannelManagerConfig.eMode =
        (uUseRedundancy != 0U) ?
            RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY :
            RSRX_REDUNDANCY_MODE_SINGLE;
    xConfig.xChannelManagerConfig.uChannelCount =
        (uUseRedundancy != 0U) ? 2U : 1U;
    xConfig.xChannelManagerConfig.axChannels[0].eChannelId =
        RSRX_TRANSPORT_CHANNEL_PRIMARY;
    xConfig.xChannelManagerConfig.axChannels[0].uPriority = 0U;
    xConfig.xChannelManagerConfig.axChannels[0].uIsAvailable = 1U;
    xConfig.xChannelManagerConfig.axChannels[1].eChannelId =
        RSRX_TRANSPORT_CHANNEL_SECONDARY;
    xConfig.xChannelManagerConfig.axChannels[1].uPriority = 1U;
    xConfig.xChannelManagerConfig.axChannels[1].uIsAvailable =
        (uUseRedundancy != 0U) ? 1U : 0U;

    if(rsrx_session_init(&pxCtx->xSession, &xConfig) != RSRX_STATUS_OK)
    {
        return -1;
    }

    if(rsrx_session_start(&pxCtx->xSession, &pxReport) != RSRX_STATUS_OK)
    {
        return -1;
    }

    if(rsrx_transport_supervisor_init(
        &pxCtx->xSupervisor,
        &pxCtx->xSession,
        rsrx_codec_get_default_port()) != RSRX_SUPERVISOR_STATUS_OK)
    {
        return -1;
    }

    return 0;
}
```

## 6. Runtime loop skeleton

```c
static void my_rasta_poll_once(my_rasta_context_t * pxCtx)
{
    const rsrx_transport_supervisor_report_t * pxSupervisorReport = 0;
    const rsrx_orchestrator_report_t * pxSessionReport = 0;
    rsrx_monotonic_time_ns_t uNowNs;

    /* 수신 frame이 있거나 주기 polling 시점이면 호출한다. */
    (void)rsrx_transport_supervisor_pump_receive(
        &pxCtx->xSupervisor,
        4U,
        &pxSupervisorReport);

    (void)pxCtx->xPlatformPorts.xClock.pfNow(
        pxCtx->xPlatformPorts.xClock.pvContext,
        &uNowNs);

    if(my_supervision_timer_expired(&pxCtx->xTimerContext, uNowNs) != 0U)
    {
        (void)rsrx_session_process_timer_expiry(
            &pxCtx->xSession,
            RSRX_TIMER_EXPIRY_SUPERVISION,
            &pxSessionReport);
    }

    if(my_retransmission_timer_expired(&pxCtx->xTimerContext, uNowNs) != 0U)
    {
        (void)rsrx_session_process_timer_expiry(
            &pxCtx->xSession,
            RSRX_TIMER_EXPIRY_RETRANSMISSION,
            &pxSessionReport);
    }

    if(my_diagnostic_timer_expired(&pxCtx->xTimerContext, uNowNs) != 0U)
    {
        (void)rsrx_session_process_timer_expiry(
            &pxCtx->xSession,
            RSRX_TIMER_EXPIRY_DIAGNOSTIC_FLUSH,
            &pxSessionReport);
    }
}
```

## 7. Client connect and application send

```c
static void my_client_start(my_rasta_context_t * pxCtx)
{
    const rsrx_orchestrator_report_t * pxReport = 0;

    (void)rsrx_session_connect(&pxCtx->xSession, &pxReport);
}

static void my_send_application_data(
    my_rasta_context_t * pxCtx,
    const uint8_t * puPayload,
    size_t xPayloadLength)
{
    if(rsrx_session_get_state(&pxCtx->xSession) != RSRX_STATE_ESTABLISHED)
    {
        return;
    }

    (void)rsrx_session_send_application_data(
        &pxCtx->xSession,
        puPayload,
        xPayloadLength);
}
```

## 8. First bring-up checklist

1. `my_transport_init()`가 `pfSend`, `pfReceive`, `pfQueryChannel`을 모두 채운다.
2. `my_platform_init()`가 `pfNow`, `pfCommand`, `pfWrite`를 모두 채운다.
3. `rsrx_session_init()`이 `RSRX_STATUS_OK`를 반환한다.
4. `rsrx_session_start()`가 `RSRX_STATUS_OK`를 반환한다.
5. client 역할에서 `rsrx_session_connect()`를 호출한다.
6. 양쪽 session이 `RSRX_STATE_ESTABLISHED`에 도달한다.
7. `rsrx_session_send_application_data()`가 `RSRX_STATUS_OK`를 반환한다.
8. 상대 application callback이 payload를 받는다.
9. timer expiry가 task/event-loop context에서 처리된다.
10. redundancy 사용 시 primary down 후 secondary traffic이 유지된다.

## 9. Copy policy

복사해서 시작해도 되는 것:

- session config 조립 순서
- channel-manager topology 설정 패턴
- receive/timer/send runtime loop 구조
- diagnostics에 남겨야 할 핵심 필드

그대로 쓰면 안 되는 것:

- POSIX socket binding
- POSIX `clock_gettime()` time source
- example stdout diagnostics
- demo용 primary down/up injection

제품 코드에서는 target driver, RTOS timer, diagnostics, link monitor로 반드시 치환해야 한다.
