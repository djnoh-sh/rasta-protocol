# AM263Px SafeRTOS Callback Reentrancy Policy Template

## Document Control

- Document ID: `EVID-TGT-007`
- Version: `0.1.0`
- Status: `Draft`
- Owner: `Project Team`
- Last Updated: `2026-06-19`

## Purpose

This template defines the callback reentrancy or deferred-callback policy that must be completed before claiming AM263Px + SafeRTOS multi-task, timer, transport callback, or ISR/deferred-ISR safety.

This is a target-policy template only. It does not claim that the policy has been implemented or executed on target.

## Scope Boundary

In scope:

1. application callback context and reentrancy policy
2. diagnostics callback context and reentrancy policy
3. timer callback entry policy
4. transport receive callback and ISR/deferred-ISR policy
5. prohibited callback-to-core call paths
6. evidence required to show callbacks do not mutate shared session state outside approved API boundaries

Out of scope:

1. changing portable core callback signatures
2. claiming ISR-safe direct core calls without target manual support
3. replacing `EVID-TGT-003` critical-section binding evidence
4. replacing target integration/fault-injection logs

## Policy Decision Matrix

| Callback / Entry Family | Expected Target Context | Direct Core API Call From Callback | Required Policy | Evidence Reference |
| --- | --- | --- | --- | --- |
| Application lifecycle callback | `TBD` | `TBD` | non-reentrant, deferred, or explicitly guarded | `TBD` |
| Application data callback | `TBD` | `TBD` | non-reentrant, deferred, or explicitly guarded | `TBD` |
| Diagnostics callback | `TBD` | `TBD` | no shared-state mutation unless deferred | `TBD` |
| SafeRTOS timer callback | `TBD` | `TBD` | direct public API boundary or deferred queue | `TBD` |
| Transport receive callback | `TBD` | `TBD` | task/deferred ISR boundary before core entry | `TBD` |
| Hardware ISR | ISR | normally no | defer by queue/task notification | `TBD` |

## Required Evidence

| Evidence ID | Required Capture | Acceptance Rule |
| --- | --- | --- |
| TGT-CB-001 | callback ownership diagram | every callback path identifies task, timer-service, ISR, or deferred-ISR context |
| TGT-CB-002 | prohibited path list | direct callback-to-core or ISR-to-core paths are either absent or explicitly justified |
| TGT-CB-003 | deferred boundary source/reference | queue/task notification/deferred handler is identified when callbacks cannot call core directly |
| TGT-CB-004 | reentrancy policy note | nested callback and callback-initiated API calls are rejected, deferred, or protected by a qualified primitive |
| TGT-CB-005 | diagnostics policy note | diagnostics callbacks cannot mutate session state or invoke core APIs unless routed through an approved boundary |
| TGT-CB-006 | fault-injection log | callback reentry attempt, queue-full deferral failure, and timer/transport concurrency scenarios preserve fail-safe behavior |
| TGT-CB-007 | integration log reference | target log shows representative callbacks under normal and fault paths without unbalanced critical-section depth |
| TGT-CB-008 | safety manual response | SafeRTOS and TI driver assumptions for callback contexts are listed with project responses |

## Minimum Target Scenarios

1. application callback does not reenter a session API directly unless the selected policy permits it
2. diagnostics callback does not mutate session state
3. timer callback path reaches the core only through the selected public API/deferred boundary
4. transport receive callback path reaches the core only through the selected public API/deferred boundary
5. ISR-originated event is deferred before core entry
6. deferred queue full condition maps to a typed diagnostic or fail-safe path
7. injected callback reentry attempt follows the selected reject/defer/guard policy

## Completion Rule

This template can be marked complete only when all `TGT-CB-*` rows are filled with target-specific artifacts and reviewed. Until then, the project may claim portable host callback behavior only, not AM263Px + SafeRTOS reentrancy safety.
