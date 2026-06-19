# POSIX Echo Example

이 예제는 `sil4/` 코어 라이브러리를 POSIX `UDP + timer` 환경에 연결하는 가장 작은 reference application이다.
목표는 "예제가 돌아간다"보다 "개발자가 우리 재구현 RASTA-PROTOCOL 기반 코드를 어떻게 붙여야 하는지 이해한다"에 있다.

## 이 예제로 이해해야 하는 것

- `rsrx_session_config_t`를 어떻게 채우는지
- transport port, clock/timer port를 실제 플랫폼에 어떻게 연결하는지
- client가 connect를 시작하고 server가 inbound connect를 받아들이는 흐름
- `ESTABLISHED` 이후 application data를 어떻게 보내고 받는지
- active-standby 이중화에서 channel failover를 어떻게 관찰하는지

구현 절차를 처음부터 따라가려면 [DEVELOPER_GUIDE.md](./DEVELOPER_GUIDE.md)를 먼저 읽는 것이 좋다. 이 README는 빌드/실행 중심이고, 개발자 가이드는 실제 제품 코드에 어떤 순서로 붙여야 하는지를 설명한다.

예제 보강 작업의 현재 계획과 진행 상태는 [EXAMPLE_ROADMAP.md](./EXAMPLE_ROADMAP.md)에서 관리한다.

## 이 예제의 비목적

- SIL4 인증 산출물 제공
- 실제 vendor CRC/MAC/timestamp parity 제공
- production-grade daemon 구조 제공

## 파일 구조

- `main.c`
  - session config 조립
  - callback 등록
  - select loop, timer expiry 처리
  - client send loop
  - failover demo용 channel down/up 주입
- `posix_transport.c`, `posix_transport.h`
  - UDP socket을 `rsrx_transport_port_t`에 연결
  - single-channel과 active-standby 2채널 endpoint 구성
- `posix_platform.c`, `posix_platform.h`
  - monotonic clock / supervision / retransmission / diagnostic timer shim
- `DEVELOPER_GUIDE.md`
  - application 코드에 transport/platform/session/runtime loop를 붙이는 단계별 구현 설명
- `EXAMPLE_ROADMAP.md`
  - 예제 문서/코드 보강 항목, 우선순위, 상태, 완료 기준 관리
- `CI_ARTIFACTS.md`
  - GitHub Actions `SIL4 Example Smoke` workflow와 `sil4-example-smoke-logs` artifact 사용 방법
- `MINIMAL_INTEGRATION.md`
  - 제품 코드에 옮길 때 필요한 최소 통합 skeleton과 확인 순서
- `PORTING_SAFERTOS_AM263PX.md`
  - SafeRTOS / AM263Px에서 POSIX 구현을 어떤 target binding으로 치환해야 하는지 설명
- `TROUBLESHOOTING.md`
  - smoke 실패, establish 실패, TX/RX 불일치, timer/failover 문제를 증상별로 진단
- `run_smoke.sh`
  - 가장 기본적인 client/server echo 검증
- `run_failover_smoke.sh`
  - active-standby failover 검증

## Build

먼저 코어 라이브러리를 빌드하고, 그 다음 example을 빌드한다.

```bash
cmake --build /tmp/sil4-build -j4
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build
```

`BUILD_DIR` 기본값은 `/tmp/sil4-build`다.

사용 가능한 target을 보려면:

```bash
make -C sil4/examples/posix_echo help
```

## 빠른 시작 1: 기본 echo

터미널 1:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build run-server
```

터미널 2:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build run-client
```

기대하는 핵심 로그:

- client: `state=2->3` 또는 `State change: 2 -> 3`
- client: `TX[1]: ...`
- server: `RX[1]: ...`
- client: `RX[1]: ...`

즉, "connect 성립 -> client send -> server receive -> server echo -> client receive" 순서만 보면 된다.

## 빠른 시작 2: failover demo

터미널 1:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build run-failover-server
```

터미널 2:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build run-failover-client
```

기대하는 핵심 로그:

- 초기: `Channel report: active=primary ...`
- failover 직후: `Injected transport event: primary forced down after rx threshold`
- 이후: `Channel report: active=secondary ... failover=1`
- 그 뒤에도 `TX[n]`, `RX[n]`가 계속 증가

이 데모는 "active channel이 primary에서 secondary로 바뀌어도 echo traffic이 유지된다"는 점을 보여준다.

## 자동 검증

기본 smoke:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build smoke
```

failover smoke:

```bash
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build failover-smoke
```

GitHub Actions에서는 `SIL4 Example Smoke` workflow를 수동 실행해서 core verification, example build, basic smoke를 재현할 수 있다.
artifact 이름은 `sil4-example-smoke-logs`다. 자세한 artifact 구성은 [CI_ARTIFACTS.md](./CI_ARTIFACTS.md)를 참고한다.

## 코드 읽는 순서

처음 보는 개발자라면 아래 순서가 가장 빠르다.

1. `main.c`에서 `xConfig`를 어떻게 채우는지 본다.
2. `xConfig.xTransportPort`, `xConfig.xPlatformPorts`가 어디서 오는지 본다.
3. `posix_transport.c`에서 `pfSend / pfReceive / pfQueryChannel`이 실제 UDP socket에 어떻게 연결되는지 본다.
4. `on_application_data()`에서 상위 application callback이 어떤 형태로 데이터를 받는지 본다.
5. failover가 필요하면 `vProcessChannelToggle()`과 `run_failover_smoke.sh`를 본다.

구체적인 시작 지점:

1. [`vBuildSessionConfig()`](./main.c)
2. [`vBuildTransportEndpoints()`](./main.c)
3. [`posix_transport_init()`](./posix_transport.c)
4. [`posix_platform_init()`](./posix_platform.c)
5. `main()` loop 안의 `vPollSupervisorReceive()`, `vProcessTimerExpiries()`, `vProcessClientSend()`

## 내 프로젝트에 붙일 때 최소 경로

이 예제를 그대로 복사할 필요는 없다. 보통은 아래 4가지만 자기 프로젝트 구조에 맞게 옮기면 된다.

1. transport binding
   - `rsrx_transport_port_t`를 채우는 코드
   - 이 예제에서는 [`posix_transport_init()`](./posix_transport.c)가 그 역할을 한다.
   - 핵심은 `pfSend`, `pfReceive`, `pfQueryChannel` 세 함수를 실제 네트워크 드라이버에 연결하는 것이다.

2. platform binding
   - `rsrx_platform_port_table_t`를 채우는 코드
   - 이 예제에서는 [`posix_platform_init()`](./posix_platform.c)가 그 역할을 한다.
   - 핵심은 monotonic clock, timer command, diagnostic write를 실제 플랫폼 서비스에 연결하는 것이다.

3. session config assembly
   - `rsrx_session_config_t`를 한 곳에서 조립하는 코드
   - 이 예제에서는 [`vBuildSessionConfig()`](./main.c)가 그 역할을 한다.
   - 핵심은 transport, codec, platform ports, callback, channel-manager topology를 한 구조체에 모으는 것이다.

4. runtime loop
   - 수신 polling, timer expiry 처리, application send trigger를 돌리는 코드
   - 이 예제에서는 `main()` loop와 `vPollSupervisorReceive()`, `vProcessTimerExpiries()`, `vProcessClientSend()`가 그 역할을 한다.

즉, 실제 프로젝트에서는 아래 순서만 만족하면 된다.

1. transport port 준비
2. platform port 준비
3. `rsrx_session_init()`
4. `rsrx_session_start()`
5. client라면 `rsrx_session_connect()`
6. event loop에서 receive + timer expiry + application send 처리

## 내 프로젝트에서 보통 바꾸는 부분

- UDP socket 대신
  - RTOS socket
  - fieldbus driver
  - proprietary link layer
- `select()` loop 대신
  - task loop
  - reactor/event loop
  - interrupt + queue 기반 dispatcher
- example 로그 출력 대신
  - 프로젝트 logger
  - event recorder
  - diagnostic buffer

반대로 아래는 보통 유지한다.

- `rsrx_session_config_t`라는 조립 지점 자체
- transport/platform을 port interface로 분리하는 구조
- `session init -> start -> connect/process event`라는 상위 흐름
- application callback에서 payload를 받아 상위 로직으로 넘기는 방식

## SafeRTOS / AM263Px 포팅

SafeRTOS / AM263Px 관점의 별도 포팅 메모는 [PORTING_SAFERTOS_AM263PX.md](./PORTING_SAFERTOS_AM263PX.md)에 분리했다.

짧게 요약하면 아래만 먼저 보면 된다.

1. POSIX `main()` loop를 SafeRTOS communication task로 치환
2. `posix_transport_init()` 역할을 AM263Px Ethernet/driver binding으로 치환
3. `posix_platform_init()` 역할을 monotonic clock / timer / diagnostics binding으로 치환
4. ISR에서는 session API를 직접 호출하지 않고 task context로 넘김
5. redundancy를 쓸 경우 `PRIMARY` / `SECONDARY` 링크 identity를 먼저 정의

## 통합 체크리스트

아래 순서대로 확인하면 "붙이긴 붙였는데 실제로는 안 도는" 상태를 줄일 수 있다.

1. transport port
   - `pfSend`, `pfReceive`, `pfQueryChannel`가 모두 연결되어 있는가
   - `pfReceive`가 payload pointer와 length를 올바르게 채우는가
   - channel-down / unavailable을 구분해서 반환하는가

2. platform port
   - monotonic clock가 ns 단위로 증가하는가
   - supervision / retransmission / diagnostic flush timer command를 처리하는가
   - diagnostics write가 최소한 로그로라도 보이는가

3. session config
   - `xTransportPort`, `xPlatformPorts`, `xCodecPort`를 모두 채웠는가
   - encode buffer와 길이를 설정했는가
   - application callback을 등록했는가
   - redundancy를 쓸 경우 channel-manager topology를 채웠는가

4. runtime flow
   - `rsrx_session_init()` 후 `rsrx_session_start()`를 호출하는가
   - client라면 `rsrx_session_connect()`를 호출하는가
   - event loop에서 receive polling을 계속 돌리는가
   - timer expiry를 session에 다시 전달하는가

5. first smoke
   - client가 `ESTABLISHED`에 들어가는가
   - client `TX[1]`가 보이는가
   - server `RX[1]`가 보이는가
   - client `RX[1]`가 보이는가

6. failover smoke
   - initial active channel이 `primary`인가
   - injected primary down 이후 active channel이 `secondary`로 바뀌는가
   - failover 뒤에도 `TX[n]`, `RX[n]`가 계속 증가하는가

## Options

```text
server|client
--local-port N
--remote-port N
--remote-ip IP
--message TEXT
--send-interval-ms N
--duration-sec N
--redundant 0|1
--local-port-secondary N
--remote-port-secondary N
--holdoff-selections N
--primary-down-at-sec N
--primary-up-at-sec N
--primary-down-after-rx N
```

## 자주 쓰는 옵션만 먼저 보면

- `--message TEXT`
  - client가 주기적으로 보낼 payload
- `--send-interval-ms N`
  - client 송신 주기
- `--duration-sec N`
  - 예제 자동 종료 시간
- `--redundant 1`
  - primary/secondary active-standby 구성 활성화
- `--primary-down-after-rx N`
  - `N`번째 receive 이후 primary down event 주입

## 동작 메모

- client는 `ESTABLISHED` 이후 주기적으로 application data를 전송한다.
- server는 받은 application data를 그대로 echo back 한다.
- remote endpoint가 예상 값과 다르면 `RX_ERROR`로 처리한다.
- 정상 종료 시 `ESTABLISHED` 상태라면 `rsrx_session_disconnect()`를 호출하고 `Graceful disconnect status=0` 로그를 남긴 뒤 transport를 정리한다.
- 이미 `SAFE_DISCONNECT`에 들어간 경우에는 중복 disconnect 없이 local cleanup으로 진행한다.
- redundancy demo 기본 secondary 포트는 server `8890`, client `8891`이다.
- `--holdoff-selections N`은 preferred recovery holdoff 길이 실험용 옵션이다.
- smoke 로그 기본 경로:
  - basic: `/tmp/rsrx-posix-echo-smoke/`
  - failover: `/tmp/rsrx-posix-echo-failover-smoke/`

## 로그 정책

이 예제의 로그는 완전한 structured logging이 아니라 개발자가 흐름을 눈으로 따라가기 쉬운 human-readable 형식을 유지한다.

유지해야 할 기준은 다음과 같다.

- smoke script가 확인하는 핵심 문자열은 안정적으로 유지한다.
- `State change`, `TX`, `RX`, `Channel report`, `Graceful disconnect`, `Stopped role` 로그는 lifecycle 이해를 위한 anchor로 둔다.
- 제품 코드에서는 이 형식을 그대로 복사하지 말고 프로젝트 diagnostics, event recorder, ring buffer, NVM snapshot 같은 표준 경로에 연결한다.
- timestamp, severity, state, reason, channel switch count 같은 필드는 target evidence 요구에 맞게 별도 logging schema로 정의한다.
