# Review Note - Protocol Context Record Gap Guard

## Document Control

- Review ID: `RV-423`
- Related Test: `TC-PC-031`
- Status: `Closed`
- Date: `2026-06-02`

## Scope

- `sil4/src/rsrx_protocol_context.c`
- `sil4/tests/unit/test_rsrx_protocol_context.c`
- `sil4/docs/verification/protocol_context_test_spec_draft.md`
- `sil4/docs/design/lld/protocol_context_lld_draft.md`
- `sil4/docs/traceability/traceability_matrix_initial.md`
- `sil4/docs/roadmap_status.md`

## Decision

`rsrx_protocol_context_record_inbound_message` now accepts sequenced inbound records only when
the record sequence is exactly `last_rx + 1`.

## Rationale

Supervisor processing already resolves gap frames as `SEQUENCE_GAP_DETECTED` and avoids recording
them as normal inbound progress. The public record API now enforces the same invariant directly:
callers cannot advance protocol context state across a missing sequence by calling record without
first resolving and recovering the gap.

Unit fixtures that previously seeded `last_rx` by recording an artificial gap now seed the baseline
through sequential records. This keeps test setup aligned with the runtime protocol invariant.

## Verification

- `TC-PC-031` now covers duplicate/lower, zero, and gap sequenced record rejection.
- Existing retransmission/recovery tests continue to seed `last_rx` through ordered records before
  exercising gap and recovery decisions.

## Residual

No new runtime residual is opened. Broader residual remains richer confirmation/retransmission
ordering variants and future message-family parity.
