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

echo "Build log: $BUILD_LOG"
echo "Cppcheck log: $CPPCHECK_LOG"
