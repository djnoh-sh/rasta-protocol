# RV-318 Codec Unsupported Reason Review

## Scope

- Review ID: `RV-318`
- Scope: `codec unsupported reason code reject`
- Date: 2026-05-04

## Findings

1. `TC-CODEC-020` verifies that outbound encode rejects a reason code outside the defined `rsrx_reason_code_t` range with `UNSUPPORTED_REASON`.
2. The same test verifies that inbound decode rejects an otherwise well-formed frame when the reason byte is outside the defined range with `UNSUPPORTED_REASON`.
3. This closes a codec boundary gap where an undefined wire reason could otherwise be forwarded as a typed reason enum, while keeping reason-code failures distinct from unsupported message-type failures.

## Disposition

- Pass.
- Codec reason-field validation now has direct encode/decode unit, spec, traceability, and roadmap evidence.
