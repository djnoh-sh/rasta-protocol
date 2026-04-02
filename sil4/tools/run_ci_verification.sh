#!/usr/bin/env bash
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-/tmp/rsrx-ci-build}"
LOG_DIR="${LOG_DIR:-/tmp/rsrx-ci-logs}"

. "$SCRIPT_DIR/verification_sequence_common.sh"

mkdir -p "$BUILD_DIR" "$LOG_DIR"
reset_verification_phase_markers "$LOG_DIR"

CONFIGURE_LOG="$LOG_DIR/configure.log"
BUILD_LOG="$LOG_DIR/build.log"
TEST_LOG="$LOG_DIR/tests.log"
CPPCHECK_LOG="$LOG_DIR/cppcheck.log"
SUMMARY_MD="$LOG_DIR/summary.md"
SUMMARY_ENV="$LOG_DIR/summary.env"

count_matches() {
  local file="$1"
  local pattern="$2"
  if [ ! -f "$file" ]; then
    echo 0
    return
  fi
  grep -Ec "$pattern" "$file" || true
}

echo "[1/4] Configure"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" >"$CONFIGURE_LOG" 2>&1
mark_verification_phase_complete "$LOG_DIR" configure

echo "[2/4] Build"
# Verification ordering rule:
# - never run test executables before the build step has fully completed
# - never overlap build, test, and cppcheck phases
require_verification_phase_complete "$LOG_DIR" configure
cmake --build "$BUILD_DIR" -j4 >"$BUILD_LOG" 2>&1
mark_verification_phase_complete "$LOG_DIR" build

echo "[3/4] Run unit/integration executables"
require_verification_phase_complete "$LOG_DIR" build
require_test_executable_ready "$BUILD_DIR/rsrx_state_machine_test"
require_test_executable_ready "$BUILD_DIR/rsrx_orchestrator_test"
require_test_executable_ready "$BUILD_DIR/rsrx_platform_contract_test"
require_test_executable_ready "$BUILD_DIR/rsrx_platform_adapters_test"
require_test_executable_ready "$BUILD_DIR/rsrx_transport_contract_test"
require_test_executable_ready "$BUILD_DIR/rsrx_channel_manager_test"
require_test_executable_ready "$BUILD_DIR/rsrx_api_test"
require_test_executable_ready "$BUILD_DIR/rsrx_config_validator_test"
require_test_executable_ready "$BUILD_DIR/rsrx_codec_contract_test"
require_test_executable_ready "$BUILD_DIR/rsrx_codec_test"
require_test_executable_ready "$BUILD_DIR/rsrx_protocol_context_test"
require_test_executable_ready "$BUILD_DIR/rsrx_transport_supervisor_test"
require_test_executable_ready "$BUILD_DIR/rsrx_session_supervisor_flow_test"
{
  "$BUILD_DIR/rsrx_state_machine_test"
  "$BUILD_DIR/rsrx_orchestrator_test"
  "$BUILD_DIR/rsrx_platform_contract_test"
  "$BUILD_DIR/rsrx_platform_adapters_test"
  "$BUILD_DIR/rsrx_transport_contract_test"
  "$BUILD_DIR/rsrx_channel_manager_test"
  "$BUILD_DIR/rsrx_api_test"
  "$BUILD_DIR/rsrx_config_validator_test"
  "$BUILD_DIR/rsrx_codec_contract_test"
  "$BUILD_DIR/rsrx_codec_test"
  "$BUILD_DIR/rsrx_protocol_context_test"
  "$BUILD_DIR/rsrx_transport_supervisor_test"
  "$BUILD_DIR/rsrx_session_supervisor_flow_test"
} >"$TEST_LOG" 2>&1
mark_verification_phase_complete "$LOG_DIR" test

echo "[4/4] Cppcheck"
require_verification_phase_complete "$LOG_DIR" test
cppcheck \
  --enable=warning,style,performance,portability \
  --std=c11 \
  --force \
  --inline-suppr \
  "$ROOT_DIR/include" \
  "$ROOT_DIR/src" \
  "$ROOT_DIR/tests/unit" \
  "$ROOT_DIR/tests/integration" \
  >"$CPPCHECK_LOG" 2>&1
mark_verification_phase_complete "$LOG_DIR" cppcheck

TEST_COUNT="$(grep -c "all tests passed" "$TEST_LOG" || true)"
CPPCHECK_FINDING_COUNT="$(grep -Evc '^(Checking |[0-9]+/[0-9]+ files checked )' "$CPPCHECK_LOG" || true)"
COMPILER_WARNING_COUNT="$(count_matches "$BUILD_LOG" 'warning:')"

SUBSET_S1_COUNT="$(count_matches "$CPPCHECK_LOG" 'nullPointer|memleak|bufferAccessOutOfBounds|outOfBounds|useAfterFree|doubleFree')"
SUBSET_S2_COUNT="$(count_matches "$CPPCHECK_LOG" 'uninitvar|unassignedVariable')"
SUBSET_S3_COUNT="$(count_matches "$CPPCHECK_LOG" 'duplicateBreak|switch.*fallthrough|missingError')"
SUBSET_S4_COUNT="$(count_matches "$CPPCHECK_LOG" 'invalidFunctionArg|invalidPointerCast|mismatch|narrowing')"
SUBSET_S5_COUNT="$(count_matches "$CPPCHECK_LOG" 'ignoredReturnValue|checkReturnValue|nullPointerRedundantCheck')"
SUBSET_S6_COUNT="$(count_matches "$CPPCHECK_LOG" 'constParameter|redundantAssignment|variableScope|unreadVariable')"

SEVERITY_CRITICAL_COUNT="$((COMPILER_WARNING_COUNT + SUBSET_S1_COUNT))"
SEVERITY_HIGH_COUNT="$((SUBSET_S2_COUNT + SUBSET_S3_COUNT + SUBSET_S4_COUNT))"
SEVERITY_MEDIUM_COUNT="$((SUBSET_S5_COUNT))"
SEVERITY_LOW_COUNT="$((SUBSET_S6_COUNT))"
SEVERITY_INFO_COUNT="0"
CONFIGURE_PHASE_MARKER="$(phase_marker_path "$LOG_DIR" configure)"
BUILD_PHASE_MARKER="$(phase_marker_path "$LOG_DIR" build)"
TEST_PHASE_MARKER="$(phase_marker_path "$LOG_DIR" test)"
CPPCHECK_PHASE_MARKER="$(phase_marker_path "$LOG_DIR" cppcheck)"
VERIFICATION_ORDERING_STATUS="Pass"

cat >"$SUMMARY_MD" <<EOF2
# SIL4 CI Summary

| Item | Result |
| --- | --- |
| Configure | Pass |
| Build | Pass |
| Test Executables Passed | ${TEST_COUNT} |
| Compiler Warning Lines | ${COMPILER_WARNING_COUNT} |
| Cppcheck Finding Lines | ${CPPCHECK_FINDING_COUNT} |
| Verification Ordering Status | ${VERIFICATION_ORDERING_STATUS} |
| Severity Mapping Reference | \`sil4/docs/evidence/severity_mapping.md\` |
| Tool-Specific Mapping Reference | \`sil4/docs/evidence/tool_specific_misra_mapping.md\` |

## Severity Buckets

| Severity | Count |
| --- | --- |
| Critical | ${SEVERITY_CRITICAL_COUNT} |
| High | ${SEVERITY_HIGH_COUNT} |
| Medium | ${SEVERITY_MEDIUM_COUNT} |
| Low | ${SEVERITY_LOW_COUNT} |
| Info | ${SEVERITY_INFO_COUNT} |

## MISRA Subset Buckets

| Subset | Count |
| --- | --- |
| MISRA-S1 Memory Safety | ${SUBSET_S1_COUNT} |
| MISRA-S2 Initialization | ${SUBSET_S2_COUNT} |
| MISRA-S3 Control Flow | ${SUBSET_S3_COUNT} |
| MISRA-S4 Interface Integrity | ${SUBSET_S4_COUNT} |
| MISRA-S5 Defensive Robustness | ${SUBSET_S5_COUNT} |
| MISRA-S6 Maintainability Style | ${SUBSET_S6_COUNT} |

## Verification Phase Markers

| Phase | Marker |
| --- | --- |
| Configure | \`$CONFIGURE_PHASE_MARKER\` |
| Build | \`$BUILD_PHASE_MARKER\` |
| Test | \`$TEST_PHASE_MARKER\` |
| Cppcheck | \`$CPPCHECK_PHASE_MARKER\` |

## Artifact Logs

- Configure: \`$CONFIGURE_LOG\`
- Build: \`$BUILD_LOG\`
- Tests: \`$TEST_LOG\`
- Cppcheck: \`$CPPCHECK_LOG\`
EOF2

cat >"$SUMMARY_ENV" <<EOF2
SUMMARY_MD=$SUMMARY_MD
CONFIGURE_LOG=$CONFIGURE_LOG
BUILD_LOG=$BUILD_LOG
TEST_LOG=$TEST_LOG
CPPCHECK_LOG=$CPPCHECK_LOG
TEST_COUNT=$TEST_COUNT
COMPILER_WARNING_COUNT=$COMPILER_WARNING_COUNT
CPPCHECK_FINDING_COUNT=$CPPCHECK_FINDING_COUNT
SEVERITY_CRITICAL_COUNT=$SEVERITY_CRITICAL_COUNT
SEVERITY_HIGH_COUNT=$SEVERITY_HIGH_COUNT
SEVERITY_MEDIUM_COUNT=$SEVERITY_MEDIUM_COUNT
SEVERITY_LOW_COUNT=$SEVERITY_LOW_COUNT
SEVERITY_INFO_COUNT=$SEVERITY_INFO_COUNT
SUBSET_S1_COUNT=$SUBSET_S1_COUNT
SUBSET_S2_COUNT=$SUBSET_S2_COUNT
SUBSET_S3_COUNT=$SUBSET_S3_COUNT
SUBSET_S4_COUNT=$SUBSET_S4_COUNT
SUBSET_S5_COUNT=$SUBSET_S5_COUNT
SUBSET_S6_COUNT=$SUBSET_S6_COUNT
VERIFICATION_ORDERING_STATUS=$VERIFICATION_ORDERING_STATUS
CONFIGURE_PHASE_MARKER=$CONFIGURE_PHASE_MARKER
BUILD_PHASE_MARKER=$BUILD_PHASE_MARKER
TEST_PHASE_MARKER=$TEST_PHASE_MARKER
CPPCHECK_PHASE_MARKER=$CPPCHECK_PHASE_MARKER
EOF2

echo "Configure log: $CONFIGURE_LOG"
echo "Build log: $BUILD_LOG"
echo "Test log: $TEST_LOG"
echo "Cppcheck log: $CPPCHECK_LOG"
echo "Verification ordering status: $VERIFICATION_ORDERING_STATUS"
echo "Summary: $SUMMARY_MD"
