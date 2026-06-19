# POSIX Echo Example Roadmap

## Document Control

- Document ID: `EXAMPLE-POSIX-ECHO-ROADMAP`
- Branch: `sil4-example-posix-echo-pr`
- Last Updated: `2026-06-19`
- Owner: `Project Team`
- Status: `Ready for PR after prerequisite core branch`

## Goal

이 예제의 목표는 개발자가 우리 재구현 RASTA-PROTOCOL 코어를 실제 application 또는 target platform에 어떻게 붙여야 하는지 빠르게 이해하게 만드는 것이다.

예제는 다음을 제공해야 한다.

- 즉시 빌드/실행 가능한 POSIX echo application
- 기본 single-channel client/server 흐름
- active-standby failover 흐름
- transport/platform/session/runtime loop 구현 경계 설명
- SafeRTOS / AM263Px 포팅 시 무엇을 치환해야 하는지에 대한 안내
- 문제가 생겼을 때 개발자가 확인할 troubleshooting 경로

## Current Baseline

| Area | Status | Evidence |
| --- | --- | --- |
| POSIX echo source | Done | `main.c`, `posix_transport.c`, `posix_platform.c` |
| Basic smoke | Done | `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build smoke` passed on `2026-06-19` after latest main alignment |
| Failover smoke | Done | `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build failover-smoke` passed on `2026-06-19` after latest main alignment |
| README quick start | Done | `README.md` |
| Developer usage guide | Done | `DEVELOPER_GUIDE.md` added in commit `dcaa280` |
| Minimal integration guide | Done | `MINIMAL_INTEGRATION.md` |
| Troubleshooting guide | Done | `TROUBLESHOOTING.md` |
| Lifecycle summary | Done | `DEVELOPER_GUIDE.md` |
| Production usage guardrails | Done | `DEVELOPER_GUIDE.md` |
| Logging policy | Done | `README.md` explains why logs stay human-readable and smoke-script stable rather than fully structured |
| Graceful shutdown example | Done | `main.c`, `DEVELOPER_GUIDE.md`, `README.md` |
| CI artifact guidance | Done | `CI_ARTIFACTS.md` |
| SafeRTOS / AM263Px porting note | Done | `PORTING_SAFERTOS_AM263PX.md` |
| Example progress tracking | Done | This document |
| Example readiness | Done | All P0/P1/P2 example items are closed as of `2026-06-19` |
| PR scope readiness | Done | This branch is based on `sil4-core-inbound-connect-prereq`; diff against that prerequisite branch contains only example/workflow/helper files |

## Work Items

| ID | Priority | Status | Item | Completion Criteria |
| --- | --- | --- | --- | --- |
| EX-PLAN-001 | P0 | Done | Maintain example roadmap | `EXAMPLE_ROADMAP.md` records current baseline, backlog, completion criteria, and verification policy |
| EX-DOC-001 | P0 | Done | Developer usage guide | `DEVELOPER_GUIDE.md` explains transport, platform, session config, runtime loop, callbacks, redundancy, timer, diagnostics, and SafeRTOS/AM263Px handoff |
| EX-DOC-002 | P0 | Done | Minimal integration guide | `MINIMAL_INTEGRATION.md` provides a compact copy-oriented integration sequence and pseudo-code |
| EX-DOC-003 | P1 | Done | Troubleshooting guide | `TROUBLESHOOTING.md` covers no establish, TX without RX, timer expiry issues, failover issues, port conflicts, and smoke log interpretation |
| EX-DOC-004 | P1 | Done | Lifecycle summary | `DEVELOPER_GUIDE.md` includes lifecycle flow, phase table, expected logs, and troubleshooting handoff |
| EX-DOC-005 | P1 | Done | Production usage guardrails | `DEVELOPER_GUIDE.md` documents forbidden/review-required patterns and recommended production structure |
| EX-CODE-001 | P2 | Done | Optional structured logging cleanup | Closed by decision: keep human-readable stable logs; full structured logging is production/project-specific and would obscure this educational example |
| EX-CODE-002 | P2 | Done | Optional graceful shutdown note/code | Example calls `rsrx_session_disconnect()` before cleanup when stopping from `ESTABLISHED`; docs explain cleanup ownership and limitations |
| EX-CI-001 | P2 | Done | CI artifact guidance | `CI_ARTIFACTS.md` documents workflow trigger, artifact contents, local log mapping, and evidence limits |
| EX-MERGE-001 | P0 | Done | Main branch merge gate | Core handshake/sequence deltas were split to `sil4-core-inbound-connect-prereq`; this branch carries only the POSIX echo example, CI workflow, and smoke helper on top of that prerequisite |

## Recommended Next Order

1. Merge or otherwise accept prerequisite branch `sil4-core-inbound-connect-prereq` first.
2. Open PR from `sil4-example-posix-echo-pr` after the prerequisite is available in the target base.
3. Re-run the example merge gate if either the prerequisite branch or `sil4-evidence-baseline` changes before merge.

## Main Branch Merge Gate

This example branch should be merged into `sil4-evidence-baseline` only after it reaches the developer-usable baseline, not merely after the executable runs.

Required criteria:

| Gate Item | Required State | Evidence |
| --- | --- | --- |
| P0 documentation | Done | `DEVELOPER_GUIDE.md`, `MINIMAL_INTEGRATION.md`, this roadmap |
| P1 usability documentation | Done | `TROUBLESHOOTING.md`, lifecycle summary, production guardrails |
| POSIX example build | Pass | `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build` passed on `2026-06-19` |
| Basic smoke | Pass | `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build smoke` passed on `2026-06-19` |
| Failover smoke | Pass | `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build failover-smoke` passed on `2026-06-19` |
| Latest main alignment | Pass with prerequisite | This branch is intentionally based on `sil4-core-inbound-connect-prereq`; example PR diff should be reviewed against that prerequisite or against `sil4-evidence-baseline` after prerequisite merge |
| Review readiness | Pass | README points to all example documents; no stale branch-only wording or missing file links |
| Scope clarity | Pass | Example remains educational/integration-oriented and does not claim SIL4 certification evidence by itself |

Current merge-readiness snapshot on `2026-06-19`:

- `cmake --build /tmp/sil4-build -j4`: pass
- `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build clean all`: pass
- `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build smoke`: pass
- `make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build failover-smoke`: pass
- Post-merge compatibility fix: POSIX platform now provides the required critical-section port; POSIX transport receive scans configured UDP channels because the core receive wrapper owns frame initialization.
- Scope split completed on `2026-06-19`: core handshake/sequence deltas were moved to `sil4-core-inbound-connect-prereq`; this branch should be compared against that prerequisite branch for example-only review.

Merge implication:

- This branch now intentionally keeps `sil4/examples/posix_echo` even though the current evidence branch baseline does not contain it.
- The runnable example depends on prerequisite core behavior in `sil4-core-inbound-connect-prereq`.
- The final example PR/merge must review the example restoration as an intentional educational addition, not as an accidental reintroduction.
- If `sil4-evidence-baseline` changes again before PR merge, rerun the merge gate and update this snapshot.

Merge procedure:

1. Finish all P0/P1 work items.
2. Update this roadmap so each completed item is marked `Done`.
3. Bring `sil4-example-posix-echo` up to date with `sil4-evidence-baseline`.
4. Run the current example verification command set.
5. Inspect `git diff --check` and final changed-file scope.
6. Open a PR or merge request from `sil4-example-posix-echo-pr` after `sil4-core-inbound-connect-prereq` is accepted or selected as the comparison base.
7. Merge only after review confirms the example does not alter core SIL4 behavior unintentionally.

Do not merge if any of the following is true:

- P0 or P1 documentation is still `Planned`.
- Smoke or failover smoke is failing.
- The prerequisite core branch has not been accepted or the PR is compared against a base that makes core deltas appear in the example PR.
- The example text implies certified target evidence or vendor-qualified evidence that this POSIX example does not provide.

## Verification Policy

Use the smallest verification that matches the change.

| Change Type | Required Verification |
| --- | --- |
| Documentation only | `git diff --check` |
| Makefile/script only | `git diff --check`, affected smoke target |
| Example source change | core build, example build, `smoke`, `failover-smoke` |
| Core SIL4 source change | follow main project rule: build, tests, cppcheck in sequence |

Current example verification command set:

```bash
cmake --build /tmp/sil4-build -j4
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build clean all
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build smoke
make -C sil4/examples/posix_echo BUILD_DIR=/tmp/sil4-build failover-smoke
```

## Definition of Done

The example branch is ready to merge into `sil4-evidence-baseline` when all P0/P1 items are `Done`, `EX-MERGE-001` is satisfied, the current example verification command set passes after aligning with the prerequisite/base branch, and PR scope contains no unreviewed core implementation delta.

Current status: ready for PR after prerequisite core branch acceptance as of `2026-06-19`.

P2 items are optional polish. They should not block sharing the example unless a reviewer identifies a concrete usability defect.

## Notes

- This branch is intentionally separate from `sil4-evidence-baseline`.
- The example is educational and integration-oriented; it is not a SIL4 certification artifact by itself.
- SafeRTOS / AM263Px production evidence remains target-specific and belongs outside this POSIX example branch unless a target-specific example is explicitly created.
