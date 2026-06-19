# POSIX Echo CI Artifact Guide

이 문서는 POSIX echo 예제의 GitHub Actions smoke workflow가 어떤 로그를 만들고, 그 로그를 어떻게 공유/검토해야 하는지 설명한다.

## Workflow

Workflow name:

```text
SIL4 Example Smoke
```

Workflow file:

```text
.github/workflows/sil4-example-smoke.yml
```

Trigger:

```text
workflow_dispatch
```

즉, GitHub Actions 화면에서 수동 실행하는 workflow다.

## What It Runs

CI workflow는 아래 순서로 실행된다.

1. checkout
2. tooling install: `cmake`, `cppcheck`, `make`, `gcc`
3. `bash sil4/tools/run_posix_echo_smoke.sh`
4. step summary publish
5. `sil4-example-smoke-logs` artifact upload

`run_posix_echo_smoke.sh` 내부 순서:

1. core verification: `sil4/tools/run_ci_verification.sh`
2. example build: `make -C sil4/examples/posix_echo BUILD_DIR=... clean all`
3. basic echo smoke: `run_smoke.sh`

중요한 범위 제한:

- CI workflow는 현재 basic echo smoke를 실행한다.
- failover smoke는 merge-gate local verification command set에 포함되어 있지만, 현재 `SIL4 Example Smoke` workflow artifact에는 포함되지 않는다.
- failover CI artifact가 필요하면 workflow와 `run_posix_echo_smoke.sh`를 별도 변경해야 한다.

## Artifact Name

Artifact name:

```text
sil4-example-smoke-logs
```

CI log root:

```text
/tmp/rsrx-example-smoke-logs
```

## Uploaded Files

| Artifact File | Meaning |
| --- | --- |
| `summary.md` | high-level example smoke result summary |
| `example_build.log` | POSIX echo example build output |
| `smoke_run.log` | `run_smoke.sh` wrapper output |
| `server.log` | basic smoke server application log |
| `client.log` | basic smoke client application log |
| `verification/configure.log` | core verification configure log |
| `verification/build.log` | core verification build log |
| `verification/tests.log` | core verification unit/integration test log |
| `verification/cppcheck.log` | core verification cppcheck log |
| `verification/summary.md` | core verification summary |
| `verification/summary.env` | machine-readable core verification status |

## Local Log Mapping

Local `make ... smoke` default logs:

```text
/tmp/rsrx-posix-echo-smoke/server.log
/tmp/rsrx-posix-echo-smoke/client.log
```

CI `run_posix_echo_smoke.sh` overrides `TMP_DIR` so the same server/client logs are written under:

```text
/tmp/rsrx-example-smoke-logs/server.log
/tmp/rsrx-example-smoke-logs/client.log
```

Local failover smoke default logs:

```text
/tmp/rsrx-posix-echo-failover-smoke/server.log
/tmp/rsrx-posix-echo-failover-smoke/client.log
```

These failover logs are not currently uploaded by `sil4-example-smoke-logs`.

## How To Use The Artifact In Review

For an example PR or merge review, attach or reference:

- GitHub Actions run URL
- `sil4-example-smoke-logs` artifact
- `summary.md`
- `verification/summary.md`
- `server.log`
- `client.log`

Minimum review questions:

1. Did core verification pass?
2. Did example build pass?
3. Did basic smoke pass?
4. Does client log show `State change: 2 -> 3`?
5. Does client log show `TX[1]` and `RX[1]`?
6. Does server log show `RX[1]` and `Echo send status=0`?

For merge into `sil4-evidence-baseline`, also keep the local failover smoke result in the PR description or review note until CI covers failover explicitly.

## What This Artifact Does Not Prove

The artifact is useful for example usability and host regression evidence, but it does not prove:

- SIL4 certification readiness
- AM263Px/SafeRTOS target behavior
- hardware CRC/crypto acceleration behavior
- failover behavior unless a separate failover log is attached
- vendor-qualified tool output
- target stack/memory/timing evidence

Those remain separate evidence workstreams.

## Recommended PR Note

Use this compact note when sharing the artifact:

```text
POSIX echo example evidence:
- Workflow: SIL4 Example Smoke
- Artifact: sil4-example-smoke-logs
- Core verification: see verification/summary.md
- Example build: see example_build.log
- Basic smoke: see smoke_run.log, server.log, client.log
- Failover smoke: verified locally unless a separate failover CI artifact is attached
```
