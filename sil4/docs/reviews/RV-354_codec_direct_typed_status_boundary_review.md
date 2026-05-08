# RV-354 Codec Direct Typed Status Boundary Review

- Review ID: `RV-354`
- Date: `2026-05-08`
- Scope: default codec direct decode status boundary.

## Findings

- The default codec direct decode path no longer returns `RSRX_CODEC_STATUS_DECODE_ERROR` for the current guard family.
- Current direct decode failures are reported through typed statuses: invalid arguments, short header, non-frame event, invalid channel, unsupported message, unsupported reason, reserved-header tamper, payload length mismatch, payload too large, CRC mismatch, and truncated CRC.
- `RSRX_CODEC_STATUS_DECODE_ERROR` remains part of the public codec contract because supervisor integration accepts alternate/custom codec ports and must preserve an unspecialized decode failure when the selected port cannot provide a narrower status.
- Existing supervisor tests intentionally keep fake-codec `DECODE_ERROR` cases to verify that generic external codec failures are preserved in supervisor reports rather than rewritten.

## Residual

- Generic direct default-codec decode collapse is closed for the current guard taxonomy.
- Residual codec/security work is future MAC/timestamp/PDU parity, broader stale/tamper taxonomy, and vendor-evidence-oriented negative vectors rather than another pass over current direct decode `DECODE_ERROR` handling.
