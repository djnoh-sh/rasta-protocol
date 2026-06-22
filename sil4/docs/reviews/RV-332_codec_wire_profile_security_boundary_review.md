# RV-332 Codec Wire-Profile Security Boundary Review

## Scope

- Review ID: `RV-332`
- Scope: codec wire-profile security-field boundary
- Date: 2026-05-06

## Findings

1. `rsrx_codec_get_wire_profile()` exposes the current fixed header size, maximum payload size, and maximum frame size through a read-only profile.
2. `TC-CODEC-024` verifies that CRC, MAC, and timestamp fields are explicitly reported as absent in the current skeleton PDU.
3. This avoids implying that the current reserved-header guard is a substitute for CRC/MAC/timestamp parity.
4. Future codec-security growth can change the profile deliberately with direct test/spec/review impact.

## Disposition

- Pass.
- The current codec-security boundary is now explicit in code, unit test, verification spec, and roadmap wording.
