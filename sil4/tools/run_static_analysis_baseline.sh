#!/usr/bin/env bash
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-/tmp/sil4-build}"
LOG_DIR="${LOG_DIR:-/tmp/rsrx-static-analysis}"

mkdir -p "$LOG_DIR"

BUILD_LOG="$LOG_DIR/build.log"
CPPCHECK_LOG="$LOG_DIR/cppcheck.log"

echo "[1/2] Build warning gate"
# Verification ordering rule:
# - cppcheck starts only after the build step has fully completed
cmake --build "$BUILD_DIR" -j4 >"$BUILD_LOG" 2>&1

echo "[2/2] Cppcheck baseline"
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

echo "Build log: $BUILD_LOG"
echo "Cppcheck log: $CPPCHECK_LOG"
