# RV-383 CRC32 Calculator Injection Review

## Scope

- `R-006` CRC32 wrapper portability
- `R-008` AM263Px hardware-backed CRC adapter seam
- `TC-CODEC-034` injected CRC32 calculator success/failure/null guard evidence

## Review Questions

1. Can target-specific CRC implementations be injected without modifying the portable codec core?
2. Does the injected calculator path preserve the same wire behavior as the default software CRC path?
3. Do injected calculator failures and null calculator misuse clear stale encode/decode outputs?

## Findings

1. `rsrx_crc32_calculate_fn` defines the portable CRC32 calculator seam.
2. `rsrx_codec_encode_message_with_crc32_calculator` and `rsrx_codec_decode_frame_with_crc32_calculator` accept an injected calculator while the existing default CRC32 APIs continue to use `rsrx_codec_calculate_crc32`.
3. `TC-CODEC-034` verifies injected calculator encode/decode round-trip equivalence using the software calculator as the injected implementation.
4. The same test verifies calculator failure propagation, stale encoded-length clear, stale decoded-message clear, and null-calculator rejection.
5. No AM263Px-specific dependency was introduced into the core codec implementation.

## Conclusion

- Pass. The codec now exposes a target-ready CRC32 calculator seam that can support AM263Px hardware CRC integration behind a target adapter while retaining the portable software fallback.

## Residual

- Actual AM263Px hardware CRC binding, software-vs-hardware equivalence logs, hardware self-test/timeout/diagnostic evidence, and target integration logs remain under `R-008`.
- MAC/timestamp/PDU parity remains future `R-006` work.
