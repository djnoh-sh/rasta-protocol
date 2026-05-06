# RV-335 Codec CRC32 Port/Profile Review

## Scope

- Review ID: `RV-335`
- Scope: CRC32 codec port and wire-profile binding
- Date: 2026-05-06

## Findings

1. `rsrx_codec_get_crc32_port()` exposes a selectable codec port bound to CRC32 encode/decode callbacks.
2. `rsrx_codec_get_crc32_wire_profile()` reports CRC present, MAC absent, timestamp absent, and the CRC-extended maximum frame size.
3. `TC-CODEC-027` verifies the CRC32 port binding and a port-level CRC32 round-trip.
4. The default codec port remains unchanged, so CRC32 adoption remains an explicit policy/configuration choice.

## Disposition

- Pass.
- CRC32 is now selectable through the same codec-port abstraction used by adapters and supervisors.
