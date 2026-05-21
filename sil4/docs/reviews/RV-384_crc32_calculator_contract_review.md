# RV-384 CRC32 Calculator Contract Review

## Scope

- `R-006` CRC32 calculator public contract
- `R-008` target CRC adapter header-level integration seam
- `TC-CODEC-035` codec contract smoke coverage

## Review Questions

1. Is `rsrx_crc32_calculate_fn` usable from the public codec header without private implementation knowledge?
2. Is the calculator signature visible to target adapter code without private codec headers?
3. Does the contract smoke test keep the new seam tied to the public header contract?

## Findings

1. `TC-CODEC-035` defines a contract-local CRC32 calculator through `rsrx_crc32_calculate_fn`.
2. The contract test assigns the local function to `rsrx_crc32_calculate_fn` and verifies the function pointer contract.
3. The test proves target adapter code can implement a calculator function with the public signature without including private codec implementation details.
4. The behavioral helper calls remain covered by `TC-CODEC-034`, which links against the codec implementation.
5. No production code change was required beyond the already-added calculator seam.

## Conclusion

- Pass. The CRC32 calculator injection seam is now covered at both behavioral unit-test level and public header contract level.

## Residual

- Actual AM263Px hardware CRC binding and software-vs-hardware equivalence evidence remain under `R-008`.
