# POSIX Echo Troubleshooting Guide

이 문서는 `sil4/examples/posix_echo` 예제를 실행하거나 제품 코드에 옮기는 중 문제가 생겼을 때 확인할 순서를 정리한다.

가장 먼저 확인할 로그:

- 기본 smoke server log: `/tmp/rsrx-posix-echo-smoke/server.log`
- 기본 smoke client log: `/tmp/rsrx-posix-echo-smoke/client.log`
- failover smoke server log: `/tmp/rsrx-posix-echo-failover-smoke/server.log`
- failover smoke client log: `/tmp/rsrx-posix-echo-failover-smoke/client.log`

## 1. 실행 파일이 없다는 메시지

증상:

```text
missing executable: .../rsrx_echo_app
```

원인:

- example을 아직 빌드하지 않았다.
- `BUILD_DIR`가 실제 core build 위치와 다르다.

확인:

```bash
cmake --build /tmp/sil4-build -j4
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build
```

조치:

- core build를 먼저 완료한다.
- 같은 `BUILD_DIR` 값으로 example을 다시 빌드한다.
- `sil4/examples/posix_echo/rsrx_echo_app`가 실행 파일인지 확인한다.

## 2. bind 실패 또는 port 충돌

증상:

```text
bind: Address already in use
```

원인:

- 이전 server/client 프로세스가 아직 살아 있다.
- 같은 포트를 다른 프로그램이 사용 중이다.
- smoke를 여러 개 동시에 실행했다.

확인:

```bash
ss -lunp | grep -E '8888|8889|8890|8891'
```

조치:

- 기존 `rsrx_echo_app` 프로세스를 종료한다.
- smoke를 동시에 여러 번 실행하지 않는다.
- 필요하면 포트를 바꿔 실행한다.

```bash
SERVER_PORT=18888 CLIENT_PORT=18889 \
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build smoke
```

## 3. client가 ESTABLISHED에 도달하지 못함

smoke 실패 메시지:

```text
[smoke] client did not reach ESTABLISHED
```

기대 로그:

```text
State change: 2 -> 3
```

가능한 원인:

- server가 먼저 떠 있지 않다.
- local/remote port가 서로 뒤집혀 있지 않다.
- remote IP가 맞지 않다.
- receive pump가 돌지 않는다.
- timer expiry가 처리되지 않아 handshake/retransmission 흐름이 멈췄다.

확인 순서:

1. server log에 `Starting server`가 있는지 본다.
2. client log에 `Starting client`가 있는지 본다.
3. server local port가 client remote port와 같은지 확인한다.
4. client local port가 server remote port와 같은지 확인한다.
5. `State change` 로그가 어느 상태까지 갔는지 확인한다.
6. diagnostics 로그에 `SAFE_DISCONNECT`, timeout, invalid argument가 있는지 확인한다.

조치:

- server를 먼저 실행한다.
- 포트 쌍을 맞춘다.
- 제품 코드에서는 receive event가 발생하지 않아도 주기적으로 receive pump 또는 driver poll을 호출한다.
- timer command를 저장만 하고 expiry를 session에 다시 넣지 않는 실수를 확인한다.

## 4. client는 TX를 했지만 server RX가 없음

smoke 실패 메시지:

```text
[smoke] server did not receive application data
```

기대 로그:

```text
client: TX[1]: ...
server: RX[1]: ...
```

가능한 원인:

- client는 send API 호출에 성공했지만 transport send가 실제 driver/link로 나가지 않았다.
- server `pfReceive`가 payload pointer 또는 length를 채우지 않았다.
- server가 예상 remote endpoint와 다른 source address를 RX_ERROR로 처리했다.
- 제품 코드에서 receive buffer 수명이 너무 짧다.

확인 순서:

1. client log에서 `TX[1]` 전후의 `application send status`를 본다.
2. server log에서 diagnostics error를 본다.
3. POSIX 예제에서는 server/client port mapping을 확인한다.
4. 제품 코드에서는 `pfSend`, `pfReceive`, `pfQueryChannel` 반환 status를 logging한다.

조치:

- `pfSend`가 실제 link로 전송하고 성공/불가/오류를 구분해서 반환하게 한다.
- `pfReceive` 성공 시 `puPayload`, `xPayloadLength`, `eEventType`을 모두 채운다.
- 수신 buffer는 supervisor 처리가 끝날 때까지 유효하게 유지한다.
- endpoint filtering을 쓰는 경우 source address/port 정책을 재확인한다.

## 5. server는 RX했지만 echo가 돌아오지 않음

smoke 실패 메시지:

```text
[smoke] server did not echo successfully
[smoke] client did not receive echo
```

기대 로그:

```text
server: RX[1]: ...
server: Echo send status=0
client: RX[1]: ...
```

가능한 원인:

- server는 application callback을 받았지만 send path가 실패했다.
- server가 아직 `ESTABLISHED`가 아니거나 safe disconnect 상태로 전이됐다.
- client receive pump가 echo frame을 처리하지 못했다.

확인 순서:

1. server log에서 `Echo send status=0`인지 확인한다.
2. `Echo send status`가 0이 아니면 transport/channel 상태를 본다.
3. client log에서 `RX_ERROR`, `SAFE_DISCONNECT`, timeout diagnostics를 본다.

조치:

- application callback 안에서 send할 때 status를 반드시 기록한다.
- 제품 코드에서는 callback에서 오래 block하지 않는다.
- client 쪽 receive pump가 send 이후에도 계속 도는지 확인한다.

## 6. timer expiry가 동작하지 않음

증상:

- handshake가 진행되다 멈춘다.
- timeout이나 retransmission이 예상대로 발생하지 않는다.
- `SAFE_DISCONNECT`가 너무 늦거나 전혀 발생하지 않는다.

가능한 원인:

- `xTimer.pfCommand`가 deadline을 저장하지 않는다.
- deadline은 저장하지만 runtime loop에서 현재 시간과 비교하지 않는다.
- RTOS timer callback에서 session API를 직접 호출하다가 task/ISR 경계가 꼬인다.
- monotonic clock가 증가하지 않거나 단위가 ns가 아니다.

확인 순서:

1. `pfNow`가 단조 증가하는 ns 값을 반환하는지 확인한다.
2. supervision/retransmission/diagnostic timer command가 각각 저장되는지 확인한다.
3. expiry 시 `rsrx_session_process_timer_expiry()`가 호출되는지 logging한다.

조치:

- POSIX 예제의 `vProcessTimerExpiries()` 구조를 기준으로 비교한다.
- SafeRTOS에서는 timer callback에서 직접 session을 호출하지 말고 communication task에 event를 보낸다.
- ns/ms/us 단위 변환을 다시 확인한다.

## 7. failover smoke에서 primary down이 주입되지 않음

failover smoke 실패 메시지:

```text
[failover-smoke] client did not inject primary down
```

기대 로그:

```text
Injected transport event: primary forced down after rx threshold
```

가능한 원인:

- `--redundant 1`이 빠졌다.
- `--primary-down-after-rx` threshold 전에 RX가 충분히 발생하지 않았다.
- client/server duration이 너무 짧다.
- application data 주기가 너무 길다.

확인 순서:

1. client 시작 로그에서 `redundant=1`인지 확인한다.
2. client `RX[n]` count가 threshold까지 증가했는지 확인한다.
3. server `RX[n]` count가 증가하는지 확인한다.

조치:

- failover smoke 기본값을 먼저 사용한다.
- 수동 실행 시 `--send-interval-ms`를 줄이거나 `--duration-sec`를 늘린다.
- 제품 코드에서는 demo injection 대신 실제 link monitor event를 supervisor에 전달한다.

## 8. failover는 발생했지만 secondary로 전환되지 않음

failover smoke 실패 메시지:

```text
[failover-smoke] client did not record failover
[failover-smoke] client did not switch to secondary
```

기대 로그:

```text
Channel report: active=secondary ... failover=1
```

가능한 원인:

- secondary channel이 channel-manager config에서 unavailable이다.
- secondary socket/link가 초기화되지 않았다.
- `pfQueryChannel`이 secondary 상태를 잘못 반환한다.
- channel down event는 들어갔지만 supervisor/channel manager가 사용할 topology가 single-channel이다.

확인 순서:

1. 시작 로그에서 `redundant=1`, secondary port가 있는지 확인한다.
2. channel report에서 `available` count를 확인한다.
3. 제품 코드에서는 `xChannelManagerConfig.uChannelCount`와 각 channel availability를 확인한다.
4. `pfQueryChannel`이 primary/secondary를 구분하는지 확인한다.

조치:

- active-standby 구성에서 secondary를 available로 시작한다.
- secondary driver/socket/link 초기화를 먼저 완료한다.
- channel ID mapping을 transport, channel manager, log에서 같은 값으로 유지한다.

## 9. failover 후 post-failover RX가 없음

failover smoke 실패 메시지:

```text
[failover-smoke] server did not receive post-failover data
[failover-smoke] client did not receive post-failover echo
```

기대 로그:

```text
server: RX[4]: ...
client: RX[4]: ...
```

가능한 원인:

- active channel은 secondary로 바뀌었지만 secondary send/receive binding이 실제로 연결되지 않았다.
- server와 client의 secondary port mapping이 맞지 않다.
- application send loop가 failover 이후 멈췄다.

확인 순서:

1. 양쪽 시작 로그에서 secondary local/remote port가 대칭인지 확인한다.
2. client channel report가 `active=secondary` 이후에도 `TX[n]`를 계속 찍는지 확인한다.
3. server log가 secondary 경로 frame을 받는지 확인한다.

조치:

- secondary endpoint를 primary와 같은 방식으로 초기화한다.
- primary/secondary remote port 쌍을 맞춘다.
- 제품 코드에서는 active channel을 application이 직접 캐시하지 말고 supervisor/channel manager가 선택하게 둔다.

## 10. 로그를 읽는 빠른 순서

기본 smoke:

1. client `State change: 2 -> 3`
2. client `TX[1]`
3. server `RX[1]`
4. server `Echo send status=0`
5. client `RX[1]`

failover smoke:

1. client `State change: 2 -> 3`
2. client/server `RX[1]`, `TX[1]`
3. client `Injected transport event: primary forced down after rx threshold`
4. client `Channel report: active=secondary ... failover=1`
5. server/client `RX[4]`

이 순서 중 처음 끊기는 지점이 실제 debugging 시작점이다.

## 11. 제품 포팅에서 자주 하는 실수

- `pfReceive`가 수신 없음과 수신 오류를 같은 status로 반환한다.
- callback payload pointer를 장기 보관한다.
- RTOS timer callback 또는 ISR에서 session API를 직접 호출한다.
- channel ID를 driver index와 혼동한다.
- monotonic time 단위를 ms로 넘기면서 ns로 착각한다.
- diagnostics callback을 비워 두어 상태 전이와 reason을 추적하지 못한다.
- active channel을 application이 따로 기억하고 있어 failover 이후에도 primary로 송신한다.

이 문제들은 대부분 `DEVELOPER_GUIDE.md`와 `MINIMAL_INTEGRATION.md`의 구조를 그대로 유지하면 피할 수 있다.
