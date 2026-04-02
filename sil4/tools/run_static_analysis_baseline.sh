#!/usr/bin/env bash
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-/tmp/sil4-build}"
LOG_DIR="${LOG_DIR:-/tmp/rsrx-static-analysis}"

. "$SCRIPT_DIR/verification_sequence_common.sh"

mkdir -p "$LOG_DIR"
reset_verification_phase_markers "$LOG_DIR"

BUILD_LOG="$LOG_DIR/build.log"
CPPCHECK_LOG="$LOG_DIR/cppcheck.log"
SUMMARY_MD="$LOG_DIR/summary.md"
SUMMARY_ENV="$LOG_DIR/summary.env"

echo "[1/2] Build warning gate"
# Verification ordering rule:
# - cppcheck starts only after the build step has fully completed
cmake --build "$BUILD_DIR" -j4 >"$BUILD_LOG" 2>&1
mark_verification_phase_complete "$LOG_DIR" build

echo "[2/2] Cppcheck baseline"
require_verification_phase_complete "$LOG_DIR" build
cppcheck \
  --enable=warning,style,performance,portability \
  --std=c11 \
  --force \
  --inline-suppr \
  "$ROOT_DIR/include" \
  "$ROOT_DIR/src" \
  "$ROOT_DIR/tests/unit" \
  "$ROOT_DIR/tests/integration" \
  2>"$CPPCHECK_LOG"
mark_verification_phase_complete "$LOG_DIR" cppcheck

BUILD_PHASE_MARKER="$(phase_marker_path "$LOG_DIR" build)"
CPPCHECK_PHASE_MARKER="$(phase_marker_path "$LOG_DIR" cppcheck)"
VERIFICATION_ORDERING_STATUS="Pass"

cat >"$SUMMARY_MD" <<EOF2
# SIL4 Static Analysis Baseline Summary

| Item | Result |
| --- | --- |
| Build Warning Gate | Pass |
| Cppcheck Baseline | Pass |
| Verification Ordering Status | ${VERIFICATION_ORDERING_STATUS} |

## Verification Phase Markers

| Phase | Marker |
| --- | --- |
| Build | \`$BUILD_PHASE_MARKER\` |
| Cppcheck | \`$CPPCHECK_PHASE_MARKER\` |

## Artifact Logs

- Build: \`$BUILD_LOG\`
- Cppcheck: \`$CPPCHECK_LOG\`
EOF2

cat >"$SUMMARY_ENV" <<EOF2
BUILD_LOG=$BUILD_LOG
CPPCHECK_LOG=$CPPCHECK_LOG
SUMMARY_MD=$SUMMARY_MD
VERIFICATION_ORDERING_STATUS=$VERIFICATION_ORDERING_STATUS
BUILD_PHASE_MARKER=$BUILD_PHASE_MARKER
CPPCHECK_PHASE_MARKER=$CPPCHECK_PHASE_MARKER
EOF2

echo "Build log: $BUILD_LOG"
echo "Cppcheck log: $CPPCHECK_LOG"
echo "Verification ordering status: $VERIFICATION_ORDERING_STATUS"
echo "Summary: $SUMMARY_MD"
