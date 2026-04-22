# Low-Level Design Draft - Channel Manager

## Document Control

- Document ID: `LLD-014`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-04-22`

## Scope

- 대상 모듈:
  - `MOD-010 Channel Manager`
- 관련 요구사항:
  - `FR-003`
  - `IF-002`
  - `SR-003`

## Responsibilities

| Element | Kind | Responsibility | Notes |
| --- | --- | --- | --- |
| `include/rsrx_channel_manager.h` | public contract | redundancy/channel selection contract 정의 | active/standby 초기 범위 |
| `src/rsrx_channel_manager.c` | implementation | 채널 상태 업데이트와 send channel 결정 | deterministic failover, preferred recovery, holdoff policy |

## Data Types

| Type | Kind | Purpose | Notes |
| --- | --- | --- | --- |
| `rsrx_redundancy_mode_t` | enum | redundancy mode 식별 | single, active-standby |
| `rsrx_channel_descriptor_t` | struct | channel id, availability, priority 보관 | config-owned descriptor |
| `rsrx_channel_manager_config_t` | struct | 채널 구성과 선호 채널 보관 | startup validated input |
| `rsrx_channel_selection_result_t` | struct | 선택 결과, failover 여부, cumulative switch telemetry 보고 | caller-visible decision |
| `rsrx_channel_manager_context_t` | struct | runtime active channel, holdoff 상태, cumulative switch count 보관 | no dynamic memory |

## Behavioral Rules

- 초기화:
  - channel count는 `1..2` 범위여야 한다.
  - `ACTIVE_STANDBY` mode는 primary/secondary topology를 구분할 수 있도록 2개 channel을 요구한다.
  - preferred channel index는 유효 범위 내여야 한다.
  - 각 channel id는 `INVALID`가 아니어야 한다.
  - configured channel id는 서로 중복될 수 없다.
  - configured channel priority는 서로 중복될 수 없다.
- runtime channel update:
  - update 대상 index의 configured channel id와 transport-reported channel id가 다르면 update를 거부한다.
  - runtime update는 channel availability만 변경하고 configured topology identity를 변경하지 않는다.
- 선택 정책:
  - `ACTIVE_STANDBY`에서는 active channel이 down이면 available한 preferred channel 또는 best available channel로 즉시 전환한다.
  - active channel이 살아 있는 상태에서 preferred channel이 복구되면 `uPreferredRecoveryHoldoffSelections`만큼 연속 관측된 뒤에만 preferred channel로 복귀한다.
  - `uPreferredRecoveryHoldoffSelections == 0`이면 preferred recovery는 즉시 수행된다.
  - 그 외에는 현재 active channel이 available이면 그대로 유지한다.
  - active channel이 unavailable이면 available channel 중 priority가 가장 높은 channel을 선택한다.
  - 새 channel이 이전 active와 다르면 `uFailoverOccurred`를 `1`로 보고한다.
  - 새 channel이 이전 active와 다를 때마다 `uTotalSwitchCount`를 증가시키고 selection result에도 현재 누적값을 복사한다.
  - 어떤 channel도 available하지 않으면 `UNAVAILABLE`을 반환한다.
- reset 정책:
  - runtime active channel을 preferred channel로 되돌린다.
  - reset은 channel availability를 변경하지 않는다.

## Safety Notes

- channel manager는 동적 메모리를 사용하지 않는다.
- channel selection은 같은 입력에 대해 같은 출력을 제공해야 한다.
- preferred channel recovery는 `ACTIVE_STANDBY`에서 holdoff 규칙을 만족한 뒤 자동 반영된다.
- channel switch 여부는 `uFailoverOccurred`로 보고되며, 현재 단계에서는 failover와 preferred recovery를 구분하지 않는다.
- cumulative switch telemetry는 reset 이후에도 유지되며, runtime 동안 발생한 failover/recovery 전환 횟수를 audit용으로 제공한다.

## Planned Verification

- `TC-CHM-001`: init/select primary contract
- `TC-CHM-002`: failover to secondary
- `TC-CHM-003`: all channels unavailable
- `TC-CHM-004`: reset to preferred channel
- `TC-CHM-005`: preferred channel recovery auto-switch
- `TC-CHM-006`: preferred recovery holdoff
- `TC-CHM-049`: invalid topology config rejection
- `TC-CHM-050`: runtime topology mutation rejection
- `TC-CHM-051`: duplicate channel priority topology rejection
